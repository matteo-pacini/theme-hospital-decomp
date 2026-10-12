#pragma once

#include "core/data/text.h"

#include <string>
#include <string_view>
#include <vector>

namespace th
{

// A line of the changelog as the title screen draws it: its text and how far
// it is indented in pixels (the lines a bullet wraps onto hang under its text).
struct ChangelogLine {
    std::string text;
    int indent = 0;
};

// `markdown` reduced to plain text, one entry per paragraph, bullet or
// heading, with an empty entry for a run of blank lines: headings are in
// capitals without their `#`, `- ` bullets are kept, a paragraph or bullet
// written over several lines is joined into one, and emphasis, code spans
// and link targets are dropped. The text of a link stays.
std::vector<std::string> changelog_paragraphs(std::string_view markdown);

// `paragraphs` broken into lines at most `width` pixels wide in `font`, at
// word boundaries (a word wider than `width` gets a line of its own). The
// lines after a bullet's first hang under its text.
std::vector<ChangelogLine> changelog_wrap(const Font &font, const std::vector<std::string> &paragraphs, int width);

// `scroll`, a pixel offset into content `content` pixels tall seen through a
// view `view` pixels tall, kept between 0 and the last offset that still fills
// the view.
int changelog_clamp_scroll(int scroll, int content, int view);

} // namespace th
