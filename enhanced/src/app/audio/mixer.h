#pragma once

#include <string>
#include <vector>

struct SDL_AudioStream;

namespace th
{

class SoundPlayer;

// Plays a SoundPlayer's voices on the default audio device. All calls,
// including the mixing, happen on the caller's thread: update() keeps a
// short queue of mixed audio ahead of the device.
class Mixer
{
  public:
    Mixer() = default;
    Mixer(const Mixer &) = delete;
    Mixer &operator=(const Mixer &) = delete;
    ~Mixer();

    // Opens the default playback device (initialising SDL's audio). On
    // failure says why in `error`; the game then runs silent.
    bool open(std::string *error = nullptr);
    void close();
    bool is_open() const { return stream_ != nullptr; }

    // Once a frame: mixes `player` into the device's queue until it holds
    // kAhead seconds of audio.
    void update(SoundPlayer &player);

    static constexpr int kRate = 44100;
    static constexpr double kAhead = 0.08;

  private:
    SDL_AudioStream *stream_ = nullptr;
    std::vector<float> buffer_;
};

} // namespace th
