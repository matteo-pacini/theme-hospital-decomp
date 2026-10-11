// The browser side of th_enhanced (linked with --pre-js): the IndexedDB
// storage under /persist, the file pickers and the report download. The C++
// side is src/app/web.cpp.

// The user's folders (src/app/paths.cpp) live in IndexedDB. The program does
// not start until they are loaded: it reads its settings and the installed
// game as the first thing it does.
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  addRunDependency('persist');
  FS.mkdir('/persist');
  FS.mount(IDBFS, {}, '/persist');
  FS.syncfs(true, function (err) {
    if (err) console.warn('storage not available, nothing is kept: ' + err);
    thFetchServerData().then(function () {
      removeRunDependency('persist');
    });
  });
});

// A page deployed with its game files (tools/web_deploy.sh --with-data, which
// sets thWithData) lists them in data/files.txt, one path per line. With no
// install in /persist/game they are fetched into /pick/data and the program
// is told to install from that folder (--drop, the event a dropped folder
// sends), so the setup screen's folder install runs instead of the picker.
// Whatever goes wrong leaves the setup screen as it is.
async function thFetchServerData() {
  if (!window.thWithData || FS.analyzePath('/persist/game/install.ok').exists) return;
  try {
    var list = await fetch('data/files.txt', { cache: 'no-cache' });
    if (!list.ok) return;
    var names = (await list.text()).split('\n').filter(Boolean);
    for (var i = 0; i < names.length; i++) {
      thStatus('Downloading ' + (i + 1) + ' of ' + names.length + ' files');
      var url = 'data/' + names[i].split('/').map(encodeURIComponent).join('/');
      var response = await fetch(url);
      if (!response.ok) throw new Error(names[i] + ': ' + response.status);
      await thWriteStream(response.body, '/pick/data/' + names[i]);
    }
    Module.arguments.push('--drop', '/pick/data');
  } catch (e) {
    console.warn('game files not loaded from the server: ' + e);
  }
  thStatus('');
}

// Writes /persist back. One write runs at a time; a request made meanwhile
// runs once more when it ends.
var thFlushing = false;
var thFlushAgain = false;
Module.thFlush = function () {
  if (thFlushing) {
    thFlushAgain = true;
    return;
  }
  thFlushing = true;
  FS.syncfs(false, function (err) {
    if (err) console.warn('storage not written: ' + err);
    thFlushing = false;
    if (thFlushAgain) {
      thFlushAgain = false;
      Module.thFlush();
    }
  });
};

Module.thRequestPersist = function () {
  if (navigator.storage && navigator.storage.persist) {
    navigator.storage.persist().catch(function () {});
  }
};

function thStatus(text) {
  var el = document.getElementById('status');
  if (!el) return;
  el.textContent = text;
  el.style.display = text ? 'block' : 'none';
}

function thMakeDirs(path) {
  var parts = path.split('/');
  var dir = '';
  for (var i = 1; i < parts.length; i++) {
    dir += '/' + parts[i];
    try { FS.mkdir(dir); } catch (e) { /* exists */ }
  }
}

// Copies a File to `dest` in memory.
function thCopyFile(file, dest) {
  return thWriteStream(file.stream(), dest);
}

// Writes a ReadableStream to `dest` in memory, a chunk at a time: an
// installer is far bigger than what is worth holding twice.
async function thWriteStream(source, dest) {
  thMakeDirs(dest.substring(0, dest.lastIndexOf('/')));
  var stream = FS.open(dest, 'w');
  var reader = source.getReader();
  var position = 0;
  for (;;) {
    var chunk = await reader.read();
    if (chunk.done) break;
    FS.write(stream, chunk.value, 0, chunk.value.length, position);
    position += chunk.value.length;
  }
  FS.close(stream);
}

// The folder's files the install reads (the manifest of
// core/data/gog_install.cpp, whatever their case), under /pick/<folder>.
async function thCopyFolder(files) {
  var wanted = new Set(ccall('th_web_manifest', 'string', [], []).split('\n'));
  var root = null;
  var chosen = [];
  for (var i = 0; i < files.length; i++) {
    var parts = files[i].webkitRelativePath.split('/');
    root = parts[0];
    if (wanted.has(parts.slice(1).join('/').toUpperCase())) chosen.push(files[i]);
  }
  for (var j = 0; j < chosen.length; j++) {
    thStatus('Copying ' + (j + 1) + ' of ' + chosen.length + ' files');
    await thCopyFile(chosen[j], '/pick/' + chosen[j].webkitRelativePath);
  }
  return root ? '/pick/' + root : null;
}

// Opens the file or folder input and tells the program (th_web_picked) the
// path of the copy, or null if nothing was chosen. The calls into the
// program are the same as SDL's dialog's callback.
Module.thPick = function (folder) {
  var input = document.createElement('input');
  input.type = 'file';
  input.style.display = 'none';
  if (folder) input.webkitdirectory = true;
  else input.accept = '.exe';
  var answered = false;
  function answer(path) {
    if (answered) return;
    answered = true;
    thStatus('');
    input.remove();
    ccall('th_web_picked', null, ['string'], [path]);
  }
  input.addEventListener('cancel', function () { answer(null); });
  input.addEventListener('change', async function () {
    if (!input.files.length) return answer(null);
    try {
      if (folder) {
        answer(await thCopyFolder(input.files));
      } else {
        var file = input.files[0];
        thStatus('Copying ' + file.name);
        await thCopyFile(file, '/pick/' + file.name);
        answer('/pick/' + file.name);
      }
    } catch (e) {
      console.warn('copy failed: ' + e);
      answer(null);
    }
  });
  document.body.appendChild(input);
  input.click();
};

// A debug report folder as one uncompressed ustar archive, downloaded.
Module.thDownloadTar = function (dir) {
  var encoder = new TextEncoder();
  var blocks = [];
  var base = dir.substring(dir.lastIndexOf('/') + 1);
  function octal(value, width) {
    return value.toString(8).padStart(width - 1, '0') + '\0';
  }
  function header(name, size) {
    var h = new Uint8Array(512);
    function put(text, offset) { h.set(encoder.encode(text), offset); }
    put(name, 0);
    put(octal(0o644, 8), 100);
    put(octal(0, 8), 108);
    put(octal(0, 8), 116);
    put(octal(size, 12), 124);
    put(octal(Math.floor(Date.now() / 1000), 12), 136);
    put('        ', 148);
    put('0', 156);
    put('ustar\0' + '00', 257);
    var sum = 0;
    for (var i = 0; i < 512; i++) sum += h[i];
    put(octal(sum, 7).slice(0, 6) + '\0 ', 148);
    return h;
  }
  FS.readdir(dir).forEach(function (name) {
    if (name === '.' || name === '..') return;
    var data = FS.readFile(dir + '/' + name);
    blocks.push(header(base + '/' + name, data.length));
    blocks.push(data);
    var pad = (512 - (data.length % 512)) % 512;
    if (pad) blocks.push(new Uint8Array(pad));
  });
  blocks.push(new Uint8Array(1024));
  var url = URL.createObjectURL(new Blob(blocks, { type: 'application/x-tar' }));
  var link = document.createElement('a');
  link.href = url;
  link.download = base + '.tar';
  document.body.appendChild(link);
  link.click();
  link.remove();
  setTimeout(function () { URL.revokeObjectURL(url); }, 10000);
};

// The program has quit: SDL's main loop is gone, and the page says so.
Module.thClosed = function () {
  document.getElementById('canvas').style.display = 'none';
  document.getElementById('status').style.display = 'none';
  document.getElementById('closed').style.display = 'block';
};
