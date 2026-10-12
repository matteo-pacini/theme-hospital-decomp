// Assembles the Vercel deploy folder for web_deploy.sh: node web_deploy.js
// DIST OUT MANIFEST [ORIG]. See that script for what the folder holds.
'use strict';

const crypto = require('crypto');
const fs = require('fs');
const path = require('path');

const [dist, out, manifestFile, orig] = process.argv.slice(2);
const webDir = path.join(__dirname, '..', 'web');

function hashed(name) {
  const data = fs.readFileSync(path.join(dist, name));
  const digest = crypto.createHash('sha256').update(data).digest('hex').slice(0, 12);
  const ext = path.extname(name);
  const target = path.basename(name, ext) + '.' + digest + ext;
  fs.writeFileSync(path.join(out, target), data);
  return target;
}

// index.js and index.wasm get their content hash in the name; the page finds
// the wasm through Module.locateFile (shell.html) and thFiles. A folder with
// data/ says so (thWithData), which bridge.js looks for.
function assemblePage(withData) {
  const files = {};
  for (const name of ['index.js', 'index.wasm']) files[name] = hashed(name);
  let page = fs.readFileSync(path.join(dist, 'index.html'), 'utf8');
  if (!page.includes('src="index.js"')) throw new Error('index.html does not load index.js');
  page = page.replace('src="index.js"', 'src="' + files['index.js'] + '"');
  const flags = 'var thFiles = ' + JSON.stringify(files) + ';' + (withData ? ' var thWithData = true; var thUpscale = "xbrz:4";' : '');
  page = page.replace('<script', '<script>' + flags + '</script>\n<script');
  fs.writeFileSync(path.join(out, 'index.html'), page);
  fs.copyFileSync(path.join(webDir, 'vercel.json'), path.join(out, 'vercel.json'));
  const others = fs.readdirSync(dist).filter((n) => !['index.html', 'index.js', 'index.wasm'].includes(n));
  if (others.length) throw new Error('unexpected files in the build output: ' + others.join(' '));
  return files;
}

// The directory entry of `dir` named `name`, whatever its case.
function entry(dir, name) {
  const want = name.toUpperCase();
  const found = fs.readdirSync(dir).find((n) => n.toUpperCase() === want);
  if (!found) throw new Error(path.join(dir, name) + ' not found');
  return path.join(dir, found);
}

// Copies the install's files, and only those (the manifest of
// core/data/gog_manifest.inc, the list the picker copies), to data/ with a
// list of them for the page.
function assembleData() {
  const text = fs.readFileSync(manifestFile, 'utf8');
  const names = [...text.matchAll(/^\s*\{"([^"]+)",/gm)].map((m) => m[1]);
  if (names.length < 300) throw new Error('manifest read wrongly: ' + names.length + ' files');
  let bytes = 0;
  for (const name of names) {
    let source = orig;
    for (const part of name.split('/')) source = entry(source, part);
    const dest = path.join(out, 'data', name);
    fs.mkdirSync(path.dirname(dest), { recursive: true });
    fs.copyFileSync(source, dest);
    bytes += fs.statSync(dest).size;
  }
  fs.writeFileSync(path.join(out, 'data', 'files.txt'), names.join('\n') + '\n');
  console.log('data: ' + names.length + ' files, ' + (bytes / 1e6).toFixed(1) + ' MB');
}

fs.rmSync(out, { recursive: true, force: true });
fs.mkdirSync(out, { recursive: true });
const files = assemblePage(Boolean(orig));
console.log('page: ' + Object.values(files).join(' '));
if (orig) assembleData();
