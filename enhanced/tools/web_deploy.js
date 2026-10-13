// Assembles the Vercel deploy folder for web_deploy.sh: node web_deploy.js
// DIST OUT. See that script for what the folder holds.
'use strict';

const crypto = require('crypto');
const fs = require('fs');
const path = require('path');

const [dist, out] = process.argv.slice(2);
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
// the wasm through Module.locateFile (shell.html) and thFiles.
function assemblePage() {
  const files = {};
  for (const name of ['index.js', 'index.wasm']) files[name] = hashed(name);
  let page = fs.readFileSync(path.join(dist, 'index.html'), 'utf8');
  if (!page.includes('src="index.js"')) throw new Error('index.html does not load index.js');
  page = page.replace('src="index.js"', 'src="' + files['index.js'] + '"');
  const flags = 'var thFiles = ' + JSON.stringify(files) + ';';
  page = page.replace('<script', '<script>' + flags + '</script>\n<script');
  fs.writeFileSync(path.join(out, 'index.html'), page);
  fs.copyFileSync(path.join(webDir, 'vercel.json'), path.join(out, 'vercel.json'));
  fs.copyFileSync(path.join(dist, 'favicon.png'), path.join(out, 'favicon.png'));
  const others = fs.readdirSync(dist).filter((n) => !['index.html', 'index.js', 'index.wasm', 'favicon.png'].includes(n));
  if (others.length) throw new Error('unexpected files in the build output: ' + others.join(' '));
  return files;
}

fs.rmSync(out, { recursive: true, force: true });
fs.mkdirSync(out, { recursive: true });
const files = assemblePage();
console.log('page: ' + Object.values(files).join(' '));
