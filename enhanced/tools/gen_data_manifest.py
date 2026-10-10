#!/usr/bin/env python3
"""Writes enhanced/src/core/data/gog_manifest.inc: where each game file the
port needs sits in GOG's installer (setup_theme_hospital_v3_(28027).exe), and
its size and sha256.

    gen_data_manifest.py INSTALLER EXTRACTED_DIR [OUT]

EXTRACTED_DIR is an innoextract extraction of INSTALLER (for example
`innoextract --language en-US -d DIR INSTALLER`). From a repository root:

    env -u LD_LIBRARY_PATH nix develop path:./nix -c sh -c \\
      'innoextract --silent --language en-US -d build/tmp-manifest "$SETUP" &&
       enhanced/tools/gen_data_manifest.py "$SETUP" build/tmp-manifest'

The installer's file data is a run of chunks, each "zlb\\x1a" followed by a
zlib stream (GOG's own helper files are LZMA chunks, skipped). A game file is
one whole chunk, or a run of whole consecutive chunks when it is large. The
script walks the chunks, inflating each, and matches each needed file's
contents against them. It fails unless every needed file is found exactly
once.
"""

import hashlib
import os
import struct
import sys
import zlib

# Bump when the set of files or what they must hold changes: an install made
# against an older manifest is then offered again.
MANIFEST_VERSION = 1

INSTALLER_SIZE = 174951008
INSTALLER_SHA256 = "3475c2bb4b262156af4d92e6b1e9e20075f1788a8776835b534e2917c7921e93"

CHUNK_MAGIC = b"zlb\x1a"
TABLE_MAGIC = b"rDlPtS"
READ = 1 << 20

# Directories installed whole, and the SOUND files needed.
WHOLE_DIRS = ("ANIMS", "DATA", "INTRO", "LEVELS", "QDATA", "QDATAM")
SOUND_PREFIXES = ("SOUND/DATA/SOUND-", "SOUND/MIDI/")
SOUND_FILES = ("SOUND/SAMPLE.OPL", "SOUND/SAMPLE.AD")


def needed(path):
    if path.split("/")[0] in WHOLE_DIRS:
        return True
    return path in SOUND_FILES or path.startswith(SOUND_PREFIXES)


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while block := f.read(READ):
            h.update(block)
    return h.hexdigest()


def data_range(f):
    """The installer's file data: from setup-1's offset to setup-0's."""
    head = f.read(READ * 4)
    at = head.find(TABLE_MAGIC)
    if at < 0:
        sys.exit("offset table not found")
    # id[12], version, total size, exe offset, exe compressed size, exe crc,
    # setup-0 offset, setup-1 offset.
    _, version, _, _, _, _, offset0, offset1 = struct.unpack_from("<12sIIIIIII", head, at)
    if version != 1:
        sys.exit("offset table version %d, expected 1" % version)
    return offset1, offset0


def inflate_chunk(f, start):
    """Inflates the zlib stream at `start`. Returns (end offset, size, sha256)."""
    d = zlib.decompressobj()
    h = hashlib.sha256()
    size = 0
    pos = start
    f.seek(start)
    while not d.eof:
        block = f.read(READ)
        if not block:
            sys.exit("chunk at %#x: stream ends in the file" % start)
        out = d.decompress(block)
        h.update(out)
        size += len(out)
        pos += len(block)
    return pos - len(d.unused_data), size, h.hexdigest()


def walk_chunks(f, first, end):
    """Lists the zlib chunks as (offset, size, sha256) and counts the others."""
    chunks = []
    other = 0
    pos = first
    while pos < end:
        f.seek(pos)
        head = f.read(6)
        if head[:4] != CHUNK_MAGIC:
            sys.exit("no chunk at %#x" % pos)
        if (head[4] << 8 | head[5]) % 31 == 0 and head[4] & 0x0F == 8:
            stop, size, digest = inflate_chunk(f, pos + 4)
            chunks.append((pos, size, digest))
            pos = stop
        else:
            other += 1
            f.seek(pos + 4)
            # No length is recorded: the chunk runs to the next magic.
            found = f.read(end - pos - 4).find(CHUNK_MAGIC)
            pos = end if found < 0 else pos + 4 + found
    if pos != end:
        sys.exit("chunks end at %#x, data at %#x" % (pos, end))
    return chunks, other


def file_sha256(path, length):
    """sha256 of the first `length` bytes of the file."""
    h = hashlib.sha256()
    with open(path, "rb") as f:
        left = length
        while left > 0:
            block = f.read(min(left, READ))
            h.update(block)
            left -= len(block)
    return h.hexdigest()


def locate(path, size, digest, chunks):
    """Returns the chunk indexes holding the file: the single chunk with its
    size and sha256, else the one run of consecutive chunks that holds it."""
    single = [i for i, c in enumerate(chunks) if c[1] == size and c[2] == digest]
    if len(single) == 1:
        return single
    if single:
        sys.exit("%s: found in %d chunks" % (path, len(single)))
    runs = []
    for i, c in enumerate(chunks):
        if c[1] >= size or file_sha256(path, c[1]) != c[2]:
            continue
        total, j = c[1], i + 1
        while total < size and j < len(chunks):
            total += chunks[j][1]
            j += 1
        if total != size:
            continue
        # The run's chunks are consecutive: the last one ends on the file's
        # end, so check the whole against the file.
        h = hashlib.sha256()
        with open(path, "rb") as g:
            pass
        runs.append((i, j))
    found = [r for r in runs if run_matches(path, digest, chunks, r)]
    if len(found) != 1:
        sys.exit("%s: found %d times" % (path, len(found)))
    return list(range(*found[0]))


def run_matches(path, digest, chunks, run):
    # Each chunk's own sha256 was compared with the file's matching slice
    # (below), so the run holds the file when every slice agrees.
    pos = 0
    with open(path, "rb") as f:
        for i in range(*run):
            _, size, chunk_digest = chunks[i]
            h = hashlib.sha256(f.read(size))
            if h.hexdigest() != chunk_digest:
                return False
            pos += size
    return pos == os.path.getsize(path)


def collect(root):
    files = []
    for dirpath, _, names in os.walk(root):
        for name in names:
            full = os.path.join(dirpath, name)
            rel = os.path.relpath(full, root).replace(os.sep, "/")
            if needed(rel):
                files.append((rel, full))
    return sorted(files)


def main():
    if len(sys.argv) not in (3, 4):
        sys.exit(__doc__)
    installer, extracted = sys.argv[1], sys.argv[2]
    out = sys.argv[3] if len(sys.argv) == 4 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..", "src", "core", "data", "gog_manifest.inc")

    if os.path.getsize(installer) != INSTALLER_SIZE or sha256_file(installer) != INSTALLER_SHA256:
        sys.exit("%s is not GOG's setup_theme_hospital_v3_(28027).exe" % installer)

    with open(installer, "rb") as f:
        first, end = data_range(f)
        chunks, other = walk_chunks(f, first, end)
    print("%d zlib chunks, %d other chunks" % (len(chunks), other), file=sys.stderr)

    entries = []
    for rel, full in collect(extracted):
        if rel != rel.upper():
            sys.exit("%s: not upper case" % rel)
        size = os.path.getsize(full)
        if size == 0:
            sys.exit("%s: empty" % rel)
        digest = sha256_file(full)
        entries.append((rel, size, digest, locate(full, size, digest, chunks)))
    if not entries:
        sys.exit("no needed files under %s" % extracted)

    with open(out, "w") as o:
        o.write("// Generated by enhanced/tools/gen_data_manifest.py from GOG's\n")
        o.write("// setup_theme_hospital_v3_(28027).exe and an innoextract extraction of it;\n")
        o.write("// do not edit. Regenerate with the command in that script's header.\n")
        o.write("// Holds only names, sizes, hashes and offsets, none of the game's data.\n\n")
        # One entry per line, as generated, rather than as clang-format packs them.
        o.write("// clang-format off\n")
        o.write("constexpr uint32_t kManifestVersion = %d;\n" % MANIFEST_VERSION)
        o.write("constexpr uint64_t kInstallerSize = %d;\n" % INSTALLER_SIZE)
        o.write('constexpr std::string_view kInstallerSha256 = "%s";\n\n' % INSTALLER_SHA256)
        used = []
        o.write("// path, size, sha256, first chunk in kManifestChunks, chunk count\n")
        o.write("constexpr ManifestFile kManifestFiles[] = {\n")
        for rel, size, digest, idx in entries:
            o.write('    {"%s", %d, "%s", %d, %d},\n' % (rel, size, digest, len(used), len(idx)))
            used.extend(idx)
        o.write("};\n\n")
        o.write("// The installer's chunks: offset of its \"zlb\\x1a\" magic, inflated size\n")
        o.write("constexpr ManifestChunk kManifestChunks[] = {\n")
        for i in used:
            o.write("    {%d, %d},\n" % (chunks[i][0], chunks[i][1]))
        o.write("};\n")
        o.write("// clang-format on\n")
    print("%d files, %d chunks -> %s" % (len(entries), len(used), out), file=sys.stderr)


if __name__ == "__main__":
    main()
