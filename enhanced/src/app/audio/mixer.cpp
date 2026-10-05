#include "app/audio/mixer.h"

#include "core/audio/music.h"
#include "core/audio/sounds.h"

#include <algorithm>

#include <SDL3/SDL.h>

namespace th
{

Mixer::~Mixer()
{
    close();
}

bool Mixer::open(std::string *error)
{
    close();
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        if (error)
            *error = SDL_GetError();
        return false;
    }
    SDL_AudioSpec spec{SDL_AUDIO_F32, 2, kRate};
    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream_) {
        if (error)
            *error = SDL_GetError();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }
    SDL_ResumeAudioStreamDevice(stream_);
    return true;
}

void Mixer::close()
{
    if (!stream_)
        return;
    SDL_DestroyAudioStream(stream_);
    stream_ = nullptr;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void Mixer::update(SoundPlayer &player, MusicPlayer *music)
{
    if (!stream_)
        return;
    const int frame_bytes = int(2 * sizeof(float));
    int queued = SDL_GetAudioStreamQueued(stream_);
    if (queued < 0)
        return;
    int frames = int(kRate * kAhead) - queued / frame_bytes;
    if (frames <= 0)
        return;
    buffer_.resize(size_t(frames) * 2);
    player.mix(buffer_.data(), frames, kRate);
    if (music) {
        music->render(buffer_.data(), frames, kRate);
        for (float &v : buffer_)
            v = std::clamp(v, -1.0f, 1.0f);
    }
    SDL_PutAudioStreamData(stream_, buffer_.data(), frames * frame_bytes);
}

} // namespace th
