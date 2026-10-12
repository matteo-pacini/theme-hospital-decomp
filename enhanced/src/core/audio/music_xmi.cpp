// The byte readers, the timbre library and the XMIDI file parser
// (src/lib/ail/ailxmidi.c, src/lib/ail/ailtimbre.c).

#include "core/audio/music_internal.h"

#include "core/data/fs.h"

#include <cstring>

namespace th
{

inline namespace music_detail
{

uint32_t be32(const uint8_t *p)
{
    return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 8 | uint32_t(p[3]);
}
uint32_t le32(const uint8_t *p)
{
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}
uint16_t le16(const uint8_t *p)
{
    return uint16_t(p[0] | p[1] << 8);
}

// XMI_read_VLN: at most 4 bytes; `end` bounds the read.
uint32_t read_vln(const uint8_t *&p, const uint8_t *end)
{
    uint32_t value = 0;
    for (int i = 0; i < 4 && p < end; i++) {
        uint8_t byte = *p++;
        value = value << 7 | (byte & 0x7f);
        if (!(byte & 0x80))
            break;
    }
    return value;
}

// XMI_message_size.
int message_size(int status)
{
    switch (status & 0xf0) {
    case 0x80:
    case 0x90:
    case 0xa0:
    case 0xb0:
    case 0xe0:
        return 3;
    case 0xc0:
    case 0xd0:
        return 2;
    }
    return 0;
}
} // namespace music_detail

// --- Files -------------------------------------------------------------------

bool TimbreBank::load(const std::filesystem::path &path, std::string *error)
{
    timbres_.clear();
    auto fail = [&](const char *why) {
        if (error)
            *error = path.string() + ": " + why;
        timbres_.clear();
        return false;
    };
    auto file = read_file(path);
    if (!file)
        return fail("cannot read");
    const std::vector<uint8_t> &d = *file;
    for (size_t at = 0;; at += 6) {
        if (at + 2 > d.size())
            return fail("no end of directory");
        if (d[at + 1] == 0xff)
            break;
        if (at + 6 > d.size())
            return fail("truncated directory");
        size_t offset = le32(&d[at + 2]);
        if (offset + 2 > d.size())
            return fail("timbre outside the file");
        size_t length = le16(&d[offset]);
        if (length < 14 || offset + length > d.size())
            return fail("bad timbre length");
        timbres_.push_back({d[at + 1], d[at], {d.begin() + long(offset), d.begin() + long(offset + length)}});
    }
    return true;
}

std::span<const uint8_t> TimbreBank::find(int bank, int patch) const
{
    for (const Timbre &t : timbres_)
        if (t.bank == bank && t.patch == patch)
            return t.data;
    return {};
}

namespace
{

// The end of the IFF chunk at `at`, or 0 if it runs past the file.
size_t chunk_end(const std::vector<uint8_t> &d, size_t at)
{
    uint64_t end = uint64_t(at) + 8 + be32(&d[at + 4]);
    return end > d.size() ? 0 : size_t(end);
}

// XMI_find_sequence, sequence 0: skip chunks until one whose form type is
// XMID; a FORM is the sequence, a CAT holds it as its first FORM XMID. Sets
// `at` to the FORM; returns the failure, or nullptr.
const char *find_sequence(const std::vector<uint8_t> &d, size_t &at)
{
    at = 0;
    for (;;) {
        if (at + 12 > d.size() || (std::memcmp(&d[at], "FORM", 4) && std::memcmp(&d[at], "CAT ", 4)))
            return "no XMIDI sequence";
        if (!std::memcmp(&d[at + 8], "XMID", 4))
            break;
        if (!chunk_end(d, at))
            return "truncated chunk";
        at = chunk_end(d, at);
    }
    if (std::memcmp(&d[at], "CAT ", 4))
        return nullptr;
    size_t end = chunk_end(d, at);
    if (!end)
        return "truncated CAT";
    at += 12;
    while (at + 12 <= end && std::memcmp(&d[at + 8], "XMID", 4)) {
        if (!chunk_end(d, at))
            return "truncated chunk";
        at = chunk_end(d, at);
    }
    if (at + 12 > end)
        return "no XMIDI sequence";
    return nullptr;
}

// Reads the TIMB, RBRN and EVNT chunks of the FORM XMID at `at` into
// `song`; returns the failure, or nullptr.
const char *read_form(size_t at, XmiSong &song)
{
    const std::vector<uint8_t> &d = song.image;
    size_t end = chunk_end(d, at);
    if (!end)
        return "truncated FORM";
    for (at += 12; at + 8 <= end;) {
        size_t next = chunk_end(d, at);
        if (!next || next > end)
            return "truncated chunk";
        if (!std::memcmp(&d[at], "TIMB", 4) && next - at >= 10) {
            size_t n = le16(&d[at + 8]);
            for (size_t i = 0; i < n && at + 12 + 2 * i <= next; i++)
                song.timbres.push_back({d[at + 11 + 2 * i], d[at + 10 + 2 * i]});
        } else if (!std::memcmp(&d[at], "RBRN", 4) && next - at >= 10) {
            song.branches = at;
        } else if (!std::memcmp(&d[at], "EVNT", 4)) {
            song.events = at + 8;
            song.events_size = next - song.events;
        }
        // Miles steps chunk by chunk without IFF's pad byte.
        at = next;
    }
    return nullptr;
}

// Walks the song's events, adding up its ticks; true if they reach an end
// of track within their chunk.
bool scan_events(XmiSong &song)
{
    const std::vector<uint8_t> &d = song.image;
    const size_t ev_end = song.events + song.events_size;
    const uint8_t *p = d.data() + song.events, *e = d.data() + ev_end;
    while (p < e) {
        uint8_t status = *p;
        if (status < 0x80) {
            song.ticks += status;
            p++;
        } else if (status == 0xff) {
            if (e - p < 2)
                return false;
            uint8_t type = p[1];
            p += 2;
            uint32_t len = read_vln(p, e);
            if (len > size_t(e - p))
                return false;
            p += len;
            if (type == 0x2f)
                return true;
        } else if (status == 0xf0 || status == 0xf7) {
            p++;
            uint32_t len = read_vln(p, e);
            if (len > size_t(e - p))
                return false;
            p += len;
        } else {
            int size = message_size(status);
            if (e - p < size)
                return false;
            p += size;
            if ((status & 0xf0) == 0x90)
                read_vln(p, e);
        }
    }
    return false;
}

} // namespace

bool parse_xmi(std::span<const uint8_t> file, XmiSong &song, std::string *error)
{
    auto fail = [&](const char *why) {
        if (error)
            *error = why;
        return false;
    };
    song = XmiSong{};
    song.image.assign(file.begin(), file.end());
    size_t at = 0;
    if (const char *why = find_sequence(song.image, at))
        return fail(why);
    if (const char *why = read_form(at, song))
        return fail(why);
    if (!song.events)
        return fail("no EVNT chunk");
    if (scan_events(song))
        return true;
    return fail("events run past their chunk without an end of track");
}

} // namespace th
