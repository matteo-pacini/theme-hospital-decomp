// The bank statement and bank manager charts of Frame: the statement's
// pages and the manager's loan, claims and face (src/finance.cpp,
// src/manager.cpp).

#include "core/ui/charts_internal.h"

#include "core/data/balance.h"
#include "core/world/finance.h"

#include <algorithm>

namespace th
{

namespace
{

// The bank manager's face animations (bank_manager_face1-3,
// src/tables.cpp): sprites by frame from 1, ended by -1.
constexpr int8_t kManagerFace1[] = {0, 7, 7, 8, 8, 8, 8, 7, 7, -1};
constexpr int8_t kManagerFace2[] = {0, 9, 9, 9, 10, 10, 10, 11, 11, 11, 11, 11, 11, 10, 10, 10, 9, 9, 9, -1};
constexpr int8_t kManagerFace3[] = {0,  12, 12, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                                    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                                    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 12, 12, -1};

// saferand (src/rand.cpp).
uint16_t saferand(uint32_t &seed)
{
    seed = seed * 0x24a1 + 0x24df;
    seed = (seed >> 13) | (seed << 19);
    return uint16_t(seed);
}

} // namespace

inline namespace charts_detail
{

void Frame::bank_statement()
{
    switch (c_.state) {
    case 0:
        load_pic();
        c_.state = 1;
        break;
    case 1:
        copy_screen();
        process_stat_panel();
        break;
    case 2:
        return_from_screen();
        break;
    }
    const int f = 0x1c;
    for (int i = 0; i < 6; i++)
        text(f, kBoxes[i + 4], {kCoords[71].x, kCoords[71].y * 2}, str(TextSection::Stat, i), kTextCentre);
    bank_text(f);
    if (exit_quick_requester())
        c_.state = 2;
}

// banktext (src/finance.cpp): the name, the lines from y 40 to 405, the
// closing balance, in font `f`.
void Frame::bank_text(int f)
{
    const Finance &fin = game_.finance;
    int line_h = line_height(font(f));
    constexpr int kName = 32, kNameY = 8, kLinesY = 40, kEndY = 405;
    const std::string &name = c_.hospital_names[0];
    text(f, kName, kNameY, name.size() > 9 ? std::string_view(name).substr(9) : std::string_view{});
    int row = 0;
    int count = fin.entry_count;
    int first = line_h ? (kEndY - kLinesY) / line_h : 0;
    first = count - first > 0 ? count - first : 0;
    int32_t balance = fin.statement_balance;
    for (int k = 0; k < first; k++)
        balance += fin.entries[size_t(k)].amount;
    for (; first < count && row < kEndY - line_h; first++) {
        if (row + kCoords[72].y > kCoords[72].y + line_h * 0x1b)
            break;
        const BankEntry &e = fin.entries[size_t(first)];
        balance += e.amount;
        bank_text_line(f, row, e, balance);
        row += line_h;
    }
    text(f, kCoords[77].x, kCoords[77].y, format("$ %7d", balance));
}

// banktext, one statement line at `row`: date, what, amount and the
// balance after it.
void Frame::bank_text_line(int f, int row, const BankEntry &e, int32_t balance)
{
    text(f, kCoords[72].x, row + kCoords[72].y, format("%d %s", e.day, std::string(text_.month(e.month)).c_str()));
    std::string what(str(TextSection::Finance, e.type));
    if (e.type == kTxInsurance)
        what += " " + std::string(str(TextSection::InsuranceNames, e.extra));
    else if (e.type == kTxTreat || e.type == kTxFinalTreat || e.type == kTxCureTreatment || e.type == kTxAdvance)
        what += " " + std::string(str(TextSection::Illness, e.extra));
    text(f, kCoords[73].x, row + kCoords[73].y, what);
    if (e.amount < 0)
        text(f, kCoords[74].x, row + kCoords[74].y, format("$ %7d", -e.amount));
    else if (e.amount != 0)
        text(f, kCoords[75].x, row + kCoords[75].y, format("$ %7d", e.amount));
    text(f, kCoords[76].x, row + kCoords[76].y, format("$ %7d", balance));
}

void Frame::process_stat_panel()
{
    if (rleft_) {
        for (int i = kStatButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                process_stat_button(i - kStatButtons);
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
            }
        }
    }
    if (mleft_) {
        for (int i = kStatButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
                break;
            }
        }
    }
    rleft_ = false;
}

void Frame::process_stat_button(int button)
{
    if (button == 0) {
        c_.state = 2;
        sound(kSfxNo4);
    } else if (button == 1) {
        sound(kSfxSelectX);
        chart_key(kChartBank);
    }
}

void Frame::display_manager()
{
    switch (c_.state) {
    case 0:
        load_pic();
        // Looked up before the chart's palette is faded in.
        c_.line_colour = find_colour(c_.previous_palette, 0x15, 0x15, 0x18);
        c_.line_shadow_colour = find_colour(c_.previous_palette, 0x2c, 0x2c, 0x2c);
        c_.state = 1;
        c_.value = 0;
        display_manager_text();
        break;
    case 1:
        copy_screen();
        display_manager_text();
        display_manager_face();
        process_bank_panel();
        break;
    case 2:
        return_from_screen();
        break;
    }
    if (rleft_) {
        const Rect &b = kBoxes[31];
        if (over(mx_, my_, b.x, b.y, b.w, b.h))
            chart_key(kChartStatement);
        if (over(mx_, my_, 0x15b, 0x198, 0x87, 0x45))
            r_.town_map = true;
    }
    left_ = mleft_ = rleft_ = rright_ = false;
    if (exit_quick_requester())
        c_.state = 2;
}

void Frame::display_manager_text()
{
    const int f = 0x1b;
    const Point at = kCoords[71];
    auto boxed = [&](int box, std::string_view s) { text(f, kBoxes[box], at, s, kTextCentre); };
    for (int i = 0; i < 5; i++)
        boxed(11 + i, str(TextSection::Bank, i));
    boxed(19, str(TextSection::Bank, 5));
    boxed(20, str(TextSection::Bank, 6));
    const Finance &fin = game_.finance;
    const Balance &bal = game_.settings();
    const Balance::Town &town = bal.towns[size_t(game_.hospital_level()) % bal.towns.size()];
    boxed(21, format("$ %d", fin.hospital_value));
    boxed(22, format("$ %d", game_.balance));
    boxed(23, format("$ %d", fin.loan));
    boxed(24, format("$ %d", int32_t(town.interest_rate) * fin.loan / 10000 / 12));
    for (int i = 0; i < 3; i++) {
        int32_t sum = 0;
        for (int32_t v : hs_.insurance_claims[size_t(i)])
            sum += v;
        boxed(25 + i, format("$ %d", sum));
    }
    boxed(28, format("%d.%d%%", town.inflation_rate / 100, town.inflation_rate % 100));
    boxed(29, format("%d.%d%%", town.interest_rate / 100, town.interest_rate % 100));
    if (c_.value) {
        reqspr(kCoords[78].x, kCoords[78].y, 1);
        c_.insurance_button_sprites = {0, 0, 2};
        size_t company = size_t(c_.value - 1);
        boxed(16, str(TextSection::InsuranceNames, game_.world.ins_control[company]));
        display_manager_claims(hs_.insurance_claims[company]);
    } else {
        c_.insurance_button_sprites = {3, 3, 3};
        for (int i = 0; i < 3; i++)
            boxed(16 + i, str(TextSection::InsuranceNames, game_.world.ins_control[size_t(i)]));
    }
}

// display_manager_text, the chosen company's graph: a year of its
// claims from this month on, scaled to the largest.
void Frame::display_manager_claims(const std::array<int32_t, 12> &claims)
{
    int32_t max = 1;
    for (int32_t v : claims)
        max = std::max(max, v);
    uint8_t black = colours().black;
    if (max > 1)
        text(3, kCoords[169].x, kCoords[169].y, format("%d", max), kTextOneColour, black);
    const Rect &g = kBoxes[30];
    int month = game_.date.month;
    int x = g.x, y = g.y - claims[size_t(month)] * g.h / max;
    int i = month, end = 12;
    for (int pass = 0; pass < 2; pass++) {
        for (; i < end; i++) {
            int px = x, py = y;
            x += g.w / 12;
            y = g.y - claims[size_t(i)] * g.h / max;
            draw_line(dst_, px + 1, py + 1, x + 1, y + 1, c_.line_shadow_colour);
            draw_line(dst_, px, py, x, y, c_.line_colour);
        }
        end = month;
        i = 0;
    }
    text(3, kCoords[170].x, kCoords[170].y, text_.month(month), kTextOneColour, black);
}

void Frame::display_manager_face()
{
    static constexpr const int8_t *kFaces[3] = {kManagerFace1, kManagerFace2, kManagerFace3};
    static constexpr int kOdds[3] = {45, 300, 300};
    for (int i = 0; i < 3; i++)
        if (saferand(c_.safeseed) % kOdds[i] == 1 && c_.face_frame[size_t(i)] == 0)
            c_.face_frame[size_t(i)] = 1;
    for (int i = 0; i < 3; i++) {
        int &frame = c_.face_frame[size_t(i)];
        if (!frame)
            continue;
        reqspr(kCoords[79 + i].x, kCoords[79 + i].y, kFaces[i][frame]);
        frame++;
        if (kFaces[i][frame] == -1) {
            frame = 0;
            if (i == 0 && saferand(c_.safeseed) % 5 == 1)
                frame = 1;
        }
    }
}

int Frame::bank_button_sprite(int i) const
{
    // display_manager_text sets the sprites of screenbuttons 5 to 7.
    return i >= 5 && i <= 7 ? c_.insurance_button_sprites[size_t(i - 5)] : kButtons[i].sprite;
}

void Frame::process_bank_panel()
{
    if (rleft_) {
        for (int i = kBankButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                process_bank_button(i - kBankButtons);
                reqspr(b.sprite_x, b.sprite_y, bank_button_sprite(i));
            }
        }
    }
    if (mleft_) {
        for (int i = kBankButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                reqspr(b.sprite_x, b.sprite_y, bank_button_sprite(i));
                break;
            }
        }
    }
}

void Frame::process_bank_button(int button)
{
    const Finance &fin = game_.finance;
    switch (button) {
    case 0:
        if (game_.balance >= 5000 && fin.loan > 0) {
            r_.event = {EV_REPAY_LOAN};
            sound(kSfxSelectX);
        } else {
            sound(kSfxWrong2);
        }
        break;
    case 1:
        if (fin.loan <= fin.max_loan - 5000) {
            r_.event = {EV_TAKE_LOAN};
            sound(kSfxSelectX);
        } else {
            sound(kSfxWrong2);
        }
        break;
    case 2:
    case 3:
        if (!c_.value) {
            c_.value = button - 1;
            sound(kSfxSelectX);
        }
        break;
    case 4:
        c_.value = c_.value ? 0 : 3;
        sound(kSfxSelectX);
        break;
    case 5:
        c_.state = 2;
        sound(kSfxNo4);
        break;
    }
}

} // namespace charts_detail

} // namespace th
