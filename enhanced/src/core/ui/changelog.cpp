#include "core/ui/changelog.h"

#include "core/render/text.h"

#include <algorithm>
#include <cctype>

namespace th
{

namespace
{

constexpr std::string_view kBullet = "- ";

std::string_view trim(std::string_view s)
{
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
        s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r'))
        s.remove_suffix(1);
    return s;
}

// `text` without `*` emphasis marks and backticks, and with each
// [label](target) as its label.
std::string strip_inline(std::string_view text)
{
    std::string out;
    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '*' || c == '`')
            continue;
        if (c == '[') {
            size_t close = text.find("](", i);
            size_t end = close == std::string_view::npos ? close : text.find(')', close);
            if (end != std::string_view::npos) {
                out += text.substr(i + 1, close - i - 1);
                i = end;
                continue;
            }
        }
        out += c;
    }
    return out;
}

// `heading` in capitals, but for a version such as v1.0.0, which keeps its
// lower case v.
std::string capitals_except_version(std::string heading)
{
    size_t pos = 0;
    while (pos < heading.size()) {
        size_t end = std::min(heading.find(' ', pos), heading.size());
        const bool version = end - pos > 1 && heading[pos] == 'v' && std::isdigit(uint8_t(heading[pos + 1]));
        for (size_t i = version ? pos + 1 : pos; i < end; i++)
            heading[i] = char(std::toupper(uint8_t(heading[i])));
        pos = end + 1;
    }
    return heading;
}

// The heading's text after its run of `#`, or an empty view if `line` is not
// a heading.
std::string_view heading_text(std::string_view line)
{
    size_t n = 0;
    while (n < line.size() && line[n] == '#')
        n++;
    if (n == 0 || n > 6 || n >= line.size() || line[n] != ' ')
        return {};
    return trim(line.substr(n));
}

// The words of `paragraph`; a bullet's "-" stays with its first word, so
// that a line never ends on it.
std::vector<std::string> words_of(const std::string &paragraph)
{
    std::vector<std::string> words;
    size_t pos = 0;
    while (pos < paragraph.size()) {
        size_t space = paragraph.find(' ', pos);
        if (space == std::string::npos)
            space = paragraph.size();
        words.push_back(paragraph.substr(pos, space - pos));
        pos = space + 1;
    }
    if (words.size() > 1 && paragraph.starts_with(kBullet)) {
        words[1] = std::string(kBullet) + words[1];
        words.erase(words.begin());
    }
    return words;
}

} // namespace

std::vector<std::string> changelog_paragraphs(std::string_view markdown)
{
    std::vector<std::string> out;
    std::string current;
    auto flush = [&] {
        if (!current.empty())
            out.push_back(strip_inline(current));
        current.clear();
    };
    auto blank = [&] {
        flush();
        if (!out.empty() && !out.back().empty())
            out.emplace_back();
    };
    size_t pos = 0;
    while (pos <= markdown.size()) {
        size_t end = markdown.find('\n', pos);
        if (end == std::string_view::npos)
            end = markdown.size();
        std::string_view line = trim(markdown.substr(pos, end - pos));
        pos = end + 1;
        if (line.empty()) {
            blank();
        } else if (std::string_view head = heading_text(line); !head.empty()) {
            flush();
            out.push_back(capitals_except_version(strip_inline(head)));
        } else if (line.starts_with(kBullet) || line.starts_with("* ")) {
            flush();
            current = std::string(kBullet) + std::string(trim(line.substr(2)));
        } else {
            if (!current.empty())
                current += ' ';
            current += line;
        }
    }
    flush();
    while (!out.empty() && out.back().empty())
        out.pop_back();
    return out;
}

std::vector<ChangelogLine> changelog_wrap(const Font &font, const std::vector<std::string> &paragraphs, int width)
{
    std::vector<ChangelogLine> lines;
    const int hang = text_width(font, kBullet);
    for (const std::string &paragraph : paragraphs) {
        if (paragraph.empty()) {
            lines.push_back({});
            continue;
        }
        const int indent = paragraph.starts_with(kBullet) ? hang : 0;
        std::string line;
        int line_indent = 0;
        for (const std::string &word : words_of(paragraph)) {
            std::string joined = line.empty() ? word : line + ' ' + word;
            if (!line.empty() && text_width(font, joined) > width - line_indent) {
                lines.push_back({line, line_indent});
                line_indent = indent;
                line = word;
            } else {
                line = std::move(joined);
            }
        }
        lines.push_back({line, line_indent});
    }
    return lines;
}

int changelog_clamp_scroll(int scroll, int content, int view)
{
    return std::clamp(scroll, 0, std::max(content - view, 0));
}

} // namespace th
