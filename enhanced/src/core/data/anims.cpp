#include "core/data/anims.h"

#include "core/data/gamedata.h"

namespace th
{

namespace
{

uint16_t le16(const std::vector<uint8_t> &d, size_t at)
{
    return uint16_t(d[at] | d[at + 1] << 8);
}

uint32_t le32(const std::vector<uint8_t> &d, size_t at)
{
    return uint32_t(le16(d, at)) | uint32_t(le16(d, at + 2)) << 16;
}

} // namespace

bool AnimData::load(const std::filesystem::path &dir, std::string *error)
{
    auto start = load_data_file(dir, "data/vstart-1.ani", error);
    auto frame = start ? load_data_file(dir, "data/vfra-1.ani", error) : std::nullopt;
    auto element = frame ? load_data_file(dir, "data/vele-1.ani", error) : std::nullopt;
    auto list = element ? load_data_file(dir, "data/vlist-1.ani", error) : std::nullopt;
    if (!list)
        return false;

    starts.clear();
    for (size_t at = 0; at + 8 <= start->size(); at += 8)
        starts.push_back({le32(*start, at), le32(*start, at + 4)});
    frames.clear();
    for (size_t at = 0; at + 10 <= frame->size(); at += 10) {
        const std::vector<uint8_t> &d = *frame;
        frames.push_back({le32(d, at), d[at + 4], d[at + 5], d[at + 6], d[at + 7], le16(d, at + 8)});
    }
    elements.clear();
    for (size_t at = 0; at + 6 <= element->size(); at += 6) {
        const std::vector<uint8_t> &d = *element;
        elements.push_back({le16(d, at), d[at + 2], d[at + 3], le16(d, at + 4)});
    }
    lists.clear();
    for (size_t at = 0; at + 2 <= list->size(); at += 2)
        lists.push_back(le16(*list, at));
    return true;
}

} // namespace th
