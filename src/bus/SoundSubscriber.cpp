#include "bus/SoundSubscriber.h"

#include <unordered_map>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

namespace {

struct GameEventTypeHash {
    size_t operator()(GameEventType type) const noexcept {
        return static_cast<size_t>(type);
    }
};

const std::unordered_map<GameEventType, const char*, GameEventTypeHash>&
soundCueByEventType() {
    static const std::unordered_map<GameEventType, const char*, GameEventTypeHash>
        kCues = {
            {GameEventType::MoveMade, "move"},
            {GameEventType::JumpMade, "jump"},
            {GameEventType::PieceCaptured, "capture"},
            {GameEventType::PiecePromoted, "promote"},
            {GameEventType::PieceSelected, "select"},
            {GameEventType::SelectionCleared, "deselect"},
            {GameEventType::GameEnded, "game_end"},
            {GameEventType::GameStarted, "game_start"},
        };
    return kCues;
}

}  // namespace

SoundSubscriber::SoundSubscriber(std::string soundsDir, std::ostream& out)
    : soundsDir_(std::move(soundsDir)), out_(out) {}

void SoundSubscriber::playCue(const char* cue) const {
    if (cue == nullptr || soundsDir_.empty()) {
        return;
    }

#ifdef _WIN32
    const std::string path = soundsDir_ + "\\" + cue + ".wav";
    PlaySoundA(path.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
#else
    (void)cue;
#endif
}

void SoundSubscriber::onEvent(const GameEvent& event) {
    const auto& cues = soundCueByEventType();
    const auto it = cues.find(event.type);
    if (it == cues.end() || it->second == nullptr) {
        return;
    }
    out_ << "[sound] trigger=" << it->second << '\n';
    playCue(it->second);
}
