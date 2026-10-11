// The browser smoke test driven by web_smoke.sh: node web_smoke.js DIST ORIG
// OUT. See that script for what it checks.
'use strict';

const fs = require('fs');
const http = require('http');
const path = require('path');
const { execFileSync } = require('child_process');
const { chromium } = require(process.env.PLAYWRIGHT_CORE);

const [dist, orig, out] = process.argv.slice(2);
const types = { '.html': 'text/html', '.js': 'text/javascript', '.wasm': 'application/wasm' };

function serve() {
  const server = http.createServer((req, res) => {
    const name = decodeURIComponent(new URL(req.url, 'http://x').pathname);
    const file = path.join(dist, name === '/' ? 'index.html' : name);
    if (!file.startsWith(dist) || !fs.existsSync(file)) {
      res.writeHead(404).end();
      return;
    }
    res.writeHead(200, {
      'Content-Type': types[path.extname(file)] || 'application/octet-stream',
      'Cross-Origin-Opener-Policy': 'same-origin',
      'Cross-Origin-Embedder-Policy': 'require-corp',
    });
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

  try {
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
    await page.evaluate(sync);
    pass('storage written');

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
