#include "core/data/fs.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <system_error>

namespace th
{

namespace
{

bool iequals(std::string_view a, std::string_view b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
               return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y));
           });
}

} // namespace

std::optional<std::filesystem::path> resolve_ci(const std::filesystem::path &root, std::string_view rel)
{
    std::filesystem::path cur = root;
    while (!rel.empty()) {
        size_t slash = rel.find('/');
        std::string_view part = rel.substr(0, slash);
        rel = slash == std::string_view::npos ? std::string_view{} : rel.substr(slash + 1);
        if (part.empty())
            continue;

        std::error_code ec;
        std::filesystem::path exact = cur / std::filesystem::path(std::string(part));
        if (std::filesystem::exists(exact, ec)) {
            cur = exact;
            continue;
        }
        bool found = false;
        for (std::filesystem::directory_iterator it(cur, ec), end; !ec && it != end; it.increment(ec)) {
            if (iequals(it->path().filename().string(), part)) {
                cur = it->path();
                found = true;
                break;
            }
        }
        if (!found)
            return std::nullopt;
    }
    return cur;
}

std::optional<std::vector<uint8_t>> read_file(const std::filesystem::path &path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::nullopt;
    in.seekg(0, std::ios::end);
    std::streamoff size = in.tellg();
    if (size < 0)
        return std::nullopt;
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    if (size > 0 && !in.read(reinterpret_cast<char *>(buf.data()), size))
        return std::nullopt;
    return buf;
}

} // namespace th
