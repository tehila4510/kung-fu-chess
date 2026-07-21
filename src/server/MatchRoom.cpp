#include "server/MatchRoom.h"

MatchRoom::MatchRoom(int matchId, UserService& users, MoveLogSubscriber* moveLog,
                     SoundSubscriber* sound)
    : matchId_(matchId), rating_(users), moveLog_(moveLog), sound_(sound) {
    if (moveLog_ != nullptr) {
        session_.bus().subscribe(moveLog_);
    }
    if (sound_ != nullptr) {
        session_.bus().subscribe(sound_);
    }
    session_.bus().subscribe(&rating_);
}

MatchRoom::~MatchRoom() {
    session_.bus().unsubscribe(&rating_);
    if (sound_ != nullptr) {
        session_.bus().unsubscribe(sound_);
    }
    if (moveLog_ != nullptr) {
        session_.bus().unsubscribe(moveLog_);
    }
}

int MatchRoom::matchId() const {
    return matchId_;
}

GameSession& MatchRoom::session() {
    return session_;
}

const GameSession& MatchRoom::session() const {
    return session_;
}

void MatchRoom::seatPlayers(ConnectionHdl white, const std::string& whiteUser,
                            ConnectionHdl black, const std::string& blackUser) {
    white_ = white;
    black_ = black;
    hasWhite_ = true;
    hasBlack_ = true;
    rating_.setSeat('W', whiteUser);
    rating_.setSeat('B', blackUser);
}

ConnectionHdl MatchRoom::whiteHdl() const {
    return white_;
}

ConnectionHdl MatchRoom::blackHdl() const {
    return black_;
}

bool MatchRoom::hasWhite() const {
    return hasWhite_;
}

bool MatchRoom::hasBlack() const {
    return hasBlack_;
}
