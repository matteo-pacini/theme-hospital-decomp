// The browser smoke test driven by web_smoke.sh: node web_smoke.js DIST ORIG
// OUT. See that script for what it checks.
'use strict';

const fs = require('fs');
const http = require('http');
const path = require('path');
const { execFileSync } = require('child_process');
const { chromium } = require(process.env.PLAYWRIGHT_CORE);

const [dist, orig, out] = process.argv.slice(2);
const types = { '.html': 'text/html', '.js': 'text/javascript', '.wasm': 'application/wasm', '.txt': 'text/plain' };

// A folder from web_deploy.js has the vercel.json that Vercel serves it with:
// its header rules are applied here, the sources as path-to-regexp patterns
// of the kinds vercel.json uses (literals, `(.*)`, `:name(regex)`). Any other
// folder gets the two isolation headers.
const config = fs.existsSync(path.join(dist, 'vercel.json')) ? JSON.parse(fs.readFileSync(path.join(dist, 'vercel.json'))) : null;
const rules = (config ? config.headers : []).map((rule) => {
  const source = rule.source.includes('(')
    ? rule.source.replace(/:[A-Za-z]+\(/g, '(')
    : rule.source.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  return { match: new RegExp('^' + source + '$'), headers: rule.headers };
});
// A folder with data/files.txt installs the game by itself.
const withData = fs.existsSync(path.join(dist, 'data', 'files.txt'));

function responseHeaders(name, file) {
  const headers = { 'Content-Type': types[path.extname(file)] || 'application/octet-stream' };
  if (!config) {
    headers['Cross-Origin-Opener-Policy'] = 'same-origin';
    headers['Cross-Origin-Embedder-Policy'] = 'require-corp';
  }
  for (const rule of rules) {
    if (rule.match.test(name)) for (const h of rule.headers) headers[h.key] = h.value;
  }
  return headers;
}

function serve() {
  const server = http.createServer((req, res) => {
    const name = decodeURIComponent(new URL(req.url, 'http://x').pathname);
    const file = path.join(dist, name === '/' ? 'index.html' : name);
    if (!file.startsWith(dist) || !fs.existsSync(file)) {
      res.writeHead(404).end();
      return;
    }
    res.writeHead(200, responseHeaders(name, file));
    fs.createReadStream(file).pipe(res);
  });
  return new Promise((resolve) => server.listen(0, '127.0.0.1', () => resolve(server)));
}

const problems = [];
function fail(message) {
  problems.push(message);
  console.log('FAIL ' + message);
}
function pass(message) {
  console.log('pass ' + message);
}

async function waitFor(page, what, fn, arg, timeout = 300000) {
  try {
    await page.waitForFunction(fn, arg, { timeout, polling: 500 });
    pass(what);
  } catch (e) {
    fail(what + ': ' + e.message.split('\n')[0]);
    throw e;
  }
}

// The runtime is up and the saved folders loaded (web/shell.html).
const ready = () => window.thReady === true;
const installed = () => Module.FS.analyzePath('/persist/game/install.ok').exists;
const sync = () => new Promise((resolve) => Module.FS.syncfs(false, resolve));

// What a deploy folder's vercel.json promises, as the requests of the page
// saw it: the isolation headers everywhere, the hashed build cached for good,
// the page and the data not.
function checkHeaders(seen) {
  const want = (pathname, key, value) => {
    const got = (seen[pathname] || {})[key.toLowerCase()];
    if (got === value) pass(pathname + ' ' + key + ': ' + value);
    else fail(pathname + ' ' + key + ': ' + got + ', wanted ' + value);
  };
  const hashed = Object.keys(seen).filter((p) => /^\/index\.[0-9a-f]{12}\.(js|wasm)$/.test(p));
  if (hashed.length !== 2) fail('hashed files requested: ' + hashed.join(' '));
  if (seen['/index.wasm']) fail('the plain index.wasm was requested');
  for (const p of ['/index.html', ...hashed]) {
    want(p, 'Cross-Origin-Opener-Policy', 'same-origin');
    want(p, 'Cross-Origin-Embedder-Policy', 'require-corp');
  }
  want('/index.html', 'Cache-Control', 'no-cache');
  for (const p of hashed) want(p, 'Cache-Control', 'public, max-age=31536000, immutable');
  if (withData) want('/data/files.txt', 'Cache-Control', 'no-cache');
}

async function main() {
  const server = await serve();
  const base = 'http://127.0.0.1:' + server.address().port + '/index.html';
  const browser = await chromium.launch({
    args: ['--use-gl=angle', '--use-angle=swiftshader', '--enable-unsafe-swiftshader', '--autoplay-policy=no-user-gesture-required'],
  });
  const context = await browser.newContext({ viewport: { width: 640, height: 480 }, acceptDownloads: true });
  const page = await context.newPage();
  page.on('console', (m) => {
    console.log('console.' + m.type() + ': ' + m.text());
    if (m.type() === 'error') fail('console error: ' + m.text());
  });
  page.on('pageerror', (e) => fail('uncaught exception: ' + e.message));

  const seen = {};
  page.on('response', (r) => {
    seen[new URL(r.url()).pathname] = r.headers();
  });
  if (withData) page.on('filechooser', () => fail('the picker was asked for in a folder with data/'));

  try {
    if (withData) {
      // The folder holds the game: it installs by itself, no picker.
      await page.goto(base);
      await waitFor(page, 'runtime started', ready, null, 120000);
      await waitFor(page, 'install finished by itself (install.ok in /persist/game)', installed);
    } else {
      // No data yet: the setup screen; its Quit button shows the closed page.
      await page.goto(base);
      await waitFor(page, 'runtime started', ready, null, 60000);
      await page.waitForTimeout(2000);
      await page.screenshot({ path: path.join(out, 'setup.png') });
      await page.mouse.click(478, 87);
      await waitFor(page, 'quit shows the closed page', () => getComputedStyle(document.getElementById('closed')).display === 'block');

      // Install from the game folder through the picker.
      await page.goto(base);
      await waitFor(page, 'runtime started again', ready, null, 60000);
      await page.waitForTimeout(2000);
      const chooser = page.waitForEvent('filechooser', { timeout: 30000 });
      await page.mouse.click(Number(process.env.SMOKE_FOLDER_X || 346), Number(process.env.SMOKE_FOLDER_Y || 87));
      await (await chooser).setFiles(orig);
      await waitFor(page, 'install finished (install.ok in /persist/game)', installed);
    }
    await page.evaluate(sync);
    pass('storage written');
    if (config) checkHeaders(seen);

    // A reload keeps the install and goes straight to level 1.
    await page.goto(base + '?level=1');
    await waitFor(page, 'runtime started after reload', ready, null, 60000);
    await waitFor(page, 'install.ok kept by the reload', installed);
    const partial = await page.evaluate(() => Module.FS.analyzePath('/persist/game.partial').exists);
    if (partial) fail('the reload started an install');
    else pass('no install on reload');
    await page.waitForTimeout(8000);
    await page.screenshot({ path: path.join(out, 'level1.png') });

    // Shift+F10: a report downloaded as one tar.
    const download = page.waitForEvent('download', { timeout: 30000 });
    await page.keyboard.press('Shift+F10');
    const tar = path.join(out, 'report.tar');
    await (await download).saveAs(tar);
    const names = execFileSync('tar', ['tf', tar]).toString().split('\n').filter(Boolean);
    const leaves = names.map((n) => path.basename(n)).sort().join(' ');
    if (leaves === 'commands.txt note.txt screenshot.png snapshot.bin state.txt') pass('report tar holds ' + leaves);
    else fail('report tar holds: ' + leaves);
  } catch (e) {
    console.log('stopped: ' + e.message.split('\n')[0]);
    await page.screenshot({ path: path.join(out, 'stopped.png') }).catch(() => {});
  }
  await browser.close();
  server.close();
  if (problems.length) {
    console.log(problems.length + ' problem(s)');
    process.exit(1);
  }
  console.log('web smoke: ok');
}

main();
