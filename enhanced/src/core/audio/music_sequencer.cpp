// The XMIDI sequencer: one sequence of Miles' MIDI driver (src/lib/ail/ailxmidi.c).

#include "core/audio/music_internal.h"

#include <algorithm>

namespace th
{

inline namespace music_detail
{

Sequencer::Sequencer(OplDriver &driver) : driver_(driver)
{
    // XMI_construct_MDI_driver's reset of every channel.
    for (int ch = 0; ch < 16; ch++) {
        driver_.message(0xb0 | ch, 114, 0);
        driver_.message(0xc0 | ch, 0, 0);
        driver_.message(0xe0 | ch, 0, 0x40);
        driver_.message(0xb0 | ch, 112, 0);
        driver_.message(0xb0 | ch, 1, 0);
        driver_.message(0xb0 | ch, 7, 127);
        driver_.message(0xb0 | ch, 10, 64);
        driver_.message(0xb0 | ch, 11, 127);
        driver_.message(0xb0 | ch, 64, 0);
        driver_.message(0xb0 | ch, 91, 40);
        driver_.message(0xb0 | ch, 93, 0);
        driver_.message(0xb0 | ch, 100, 0);
        driver_.message(0xb0 | ch, 101, 0);
        driver_.message(0xb0 | ch, 38, 0);
        driver_.message(0xb0 | ch, 6, 12);
    }
}

bool Sequencer::start(const XmiSong &song, const TimbreBank &bank, int volume)
{
    end();
    song_ = &song;
    loop_count_ = 1;
    volume_ = volume_target_ = 127;
    volume_period_ = volume_accum_ = 0;
    tempo_error_ = 0;
    // XMI_rewind_sequence.
    shadow_volume_.fill(-1);
    shadow_sustain_.fill(-1);
    shadow_voice_protect_.fill(-1);
    for_count_.fill(-1);
    note_chan_.fill(-1);
    note_count_ = 0;
    interval_count_ = 0;
    interval_num_ = 0;
    pos_ = song.events;
    bool ok = song.timbres.empty() || driver_.install_set(song.timbres, bank);
    // PlayMidiFromAddress sets a volume below full at once.
    if (volume < 127)
        set_volume(volume, 0);
    status_ = Playing;
    return ok;
}

void Sequencer::flush_notes()
{
    for (int i = 0; i < kMaxNotes; i++) {
        if (note_chan_[size_t(i)] == -1)
            continue;
        send(note_chan_[size_t(i)] | 0x80, note_num_[size_t(i)], 0);
        note_chan_[size_t(i)] = -1;
    }
    note_count_ = 0;
}

// AIL_stop_sequence.
void Sequencer::stop()
{
    if (status_ != Playing)
        return;
    status_ = Stopped;
    flush_notes();
    for (int ch = 0; ch < 16; ch++) {
        if (shadow_sustain_[size_t(ch)] >= 64)
            driver_.message(0xb0 | ch, 64, 0);
        if (shadow_voice_protect_[size_t(ch)] >= 64)
            driver_.message(0xb0 | ch, 112, 0);
    }
}

// AIL_resume_sequence. Its XMI_refresh_channel resends every controller the
// sequence set; the driver still holds them all, nothing having played
// since, but for the sustain and voice protection stop() released.
void Sequencer::resume()
{
    if (status_ != Stopped)
        return;
    for (int ch = 0; ch < 16; ch++) {
        if (shadow_voice_protect_[size_t(ch)] != -1)
            driver_.message(0xb0 | ch, 112, shadow_voice_protect_[size_t(ch)]);
        if (shadow_sustain_[size_t(ch)] != -1)
            driver_.message(0xb0 | ch, 64, shadow_sustain_[size_t(ch)]);
    }
    status_ = Playing;
}

// AIL_end_sequence.
void Sequencer::end()
{
    if (status_ == Done)
        return;
    stop();
    status_ = Done;
}

void Sequencer::set_volume(int volume, int milliseconds)
{
    volume_target_ = volume;
    if (volume_ == volume_target_)
        return;
    if (milliseconds == 0) {
        volume_ = volume_target_;
    } else {
        volume_accum_ = 0;
        volume_period_ = milliseconds * 1000 / std::abs(volume_ - volume_target_);
    }
    update_volume();
}

// XMI_update_volume.
void Sequencer::update_volume()
{
    for (int ch = 0; ch < 16; ch++)
        if (shadow_volume_[size_t(ch)] != -1)
            send(0xb0 | ch, 7, shadow_volume_[size_t(ch)]);
}

// XMI_send_channel_voice_message, for one sequence on unlocked channels.
void Sequencer::send(int status, int data1, int data2)
{
    const int st = status & 0xf0, ch = status & 0x0f;
    if (st == 0xb0) {
        switch (data1) {
        case 7:
            shadow_volume_[size_t(ch)] = int8_t(data2);
            data2 = std::clamp(volume_ * data2 * 127 / (127 * 127), 0, 127);
            break;
        case 64:
            shadow_sustain_[size_t(ch)] = int8_t(data2);
            break;
        case 112:
            shadow_voice_protect_[size_t(ch)] = int8_t(data2);
            break;
        case 6: // the bend range goes out with its RPN select
            driver_.message(0xb0 | ch, 100, 0);
            driver_.message(0xb0 | ch, 101, 0);
            driver_.message(0xb0 | ch, 38, 0);
            break;
        case 116: // FOR_LOOP
            for_loop(data2);
            return;
        case 117: // NEXT_LOOP
            next_loop(data2);
            return;
        case 109: // SEQ_BRANCH: AIL_branch_index
            branch(data2);
            return;
        case 108: // CALLBACK_PFX
        case 110: // CHAN_LOCK
        case 111: // CHAN_PROTECT
        case 115: // INDIRECT_C_PFX
        case 118: // CLEAR_BEAT_BAR
        case 119: // CALLBACK_TRIG
            return;
        default:
            break;
        }
    }
    driver_.message(status, data1, data2);
}

// FOR_LOOP: opens a loop of `count` passes in the first free nesting level.
void Sequencer::for_loop(int count)
{
    for (int i = 0; i < kForNest; i++) {
        if (for_count_[size_t(i)] == -1) {
            for_count_[size_t(i)] = count;
            for_pos_[size_t(i)] = pos_;
            break;
        }
    }
}

// NEXT_LOOP: for a value of 64 or more, jumps back to the innermost loop's
// start, or closes it after its last pass.
void Sequencer::next_loop(int value)
{
    if (value < 64)
        return;
    int i = kForNest - 1;
    while (i >= 0 && for_count_[size_t(i)] == -1)
        i--;
    if (i < 0)
        return;
    if (for_count_[size_t(i)] == 0 || --for_count_[size_t(i)] != 0)
        pos_ = for_pos_[size_t(i)];
    else
        for_count_[size_t(i)] = -1;
}

// AIL_branch_index: jumps to the RBRN entry `index`, if the song has it.
void Sequencer::branch(int index)
{
    if (!song_->branches)
        return;
    const uint8_t *r = song_->image.data() + song_->branches;
    int n = int16_t(le16(r + 8));
    for (int i = 0; i < n; i++) {
        const uint8_t *e = r + 10 + 6 * i;
        if (le16(e) == index && le32(e + 2) < song_->events_size) {
            pos_ = song_->events + le32(e + 2);
            interval_count_ = 0;
            for_count_.fill(-1);
            break;
        }
    }
}

void Sequencer::serve()
{
    if (status_ != Playing)
        return;
    interval_num_++;
    const uint8_t *data = song_->image.data();
    const size_t end = song_->events + song_->events_size;
    bool done = false;
    tempo_error_ += 100;
    while (tempo_error_ >= 100) {
        tempo_error_ -= 100;
        expire_notes();
        if (--interval_count_ > 0)
            continue;
        const Events events = play_events();
        if (events == Events::Overflow)
            return;
        if (events == Events::Done) {
            done = true;
            break;
        }
        interval_count_ = pos_ < end ? data[pos_++] : 0;
    }
    if (done || status_ != Playing)
        return;
    fade_volume();
}

// serve's note timers: ends each note whose time has run out.
void Sequencer::expire_notes()
{
    for (int i = 0; i < kMaxNotes && note_count_ > 0; i++) {
        if (note_chan_[size_t(i)] == -1 || --note_time_[size_t(i)] > 0)
            continue;
        send(note_chan_[size_t(i)] | 0x80, note_num_[size_t(i)], 0);
        note_chan_[size_t(i)] = -1;
        note_count_--;
    }
}

// serve's events up to the next delay: Wait at a delay byte, Done at the end
// of the song, Overflow when the note queue overflowed and the sequence
// ended.
Sequencer::Events Sequencer::play_events()
{
    const uint8_t *data = song_->image.data();
    const size_t end = song_->events + song_->events_size;
    for (;;) {
        // parse_xmi checked the events up to the end of track; a jump
        // elsewhere may still run off the chunk, taken as its end.
        const int status = pos_ < end ? data[pos_] : 0xff;
        if (status < 0x80)
            return Events::Wait;
        if (pos_ >= end || status == 0xff) {
            if (meta_event())
                return Events::Done;
        } else if (status == 0xf0 || status == 0xf7) {
            // The driver ignores system exclusive messages.
            const uint8_t *p = data + pos_ + 1;
            uint32_t len = read_vln(p, data + end);
            pos_ = size_t(p - data) + len;
        } else if (!channel_event(status)) {
            return Events::Overflow;
        }
    }
}

// A meta event at pos_ (or the chunk's end): skipped, but for the end of
// track, which loops the song or ends it, returning true.
bool Sequencer::meta_event()
{
    const uint8_t *data = song_->image.data();
    const size_t end = song_->events + song_->events_size;
    int type = 0x2f;
    if (pos_ < end) {
        const uint8_t *p = data + pos_ + 2;
        type = pos_ + 1 < end ? data[pos_ + 1] : 0x2f;
        uint32_t len = read_vln(p, data + end);
        pos_ = size_t(p - data) + len;
    }
    if (type != 0x2f)
        return false;
    if (loop_count_ == 0 || --loop_count_ != 0) {
        pos_ = song_->events;
    } else {
        loop_count_ = 1;
        stop();
        status_ = Done;
    }
    return true;
}

// A channel message at pos_, sent; a note-on is queued for its note-off.
// False when the note queue overflows, which ends the sequence.
bool Sequencer::channel_event(int status)
{
    const uint8_t *data = song_->image.data();
    const size_t end = song_->events + song_->events_size;
    const int size = message_size(status);
    if (pos_ + size_t(size) > end) {
        pos_ = end;
        return true;
    }
    const size_t at = pos_;
    pos_ += size_t(size);
    send(status, data[at + 1], size == 3 ? data[at + 2] : 0);
    if ((status & 0xf0) != 0x90)
        return true;
    int slot = 0;
    while (slot < kMaxNotes && note_chan_[size_t(slot)] != -1)
        slot++;
    if (slot == kMaxNotes) {
        // "Internal note queue overflow".
        stop();
        status_ = Done;
        return false;
    }
    note_count_++;
    note_chan_[size_t(slot)] = status & 0x0f;
    note_num_[size_t(slot)] = data[at + 1];
    const uint8_t *p = data + pos_;
    note_time_[size_t(slot)] = int(read_vln(p, data + end));
    pos_ = size_t(p - data);
    return true;
}

// serve's volume fade towards volume_target_.
void Sequencer::fade_volume()
{
    if (volume_ == volume_target_)
        return;
    volume_accum_ += kIntervalTime;
    while (volume_accum_ >= volume_period_) {
        volume_accum_ -= volume_period_;
        volume_ += volume_target_ > volume_ ? 1 : -1;
        if (volume_ == volume_target_)
            break;
    }
    // Only every eighth service resends the volume, so a fade can end a few
    // steps short of its target.
    if (!(interval_num_ & 7))
        update_volume();
}

} // namespace music_detail

} // namespace th
