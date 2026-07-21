#ifndef MATCH_ROOM_H
#define MATCH_ROOM_H

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#ifndef _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_STL_
#endif
#ifndef _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_CPP11_THREAD_
#endif

#include "auth/UserService.h"
#include "bus/MoveLogSubscriber.h"
#include "bus/RatingSubscriber.h"
#include "bus/SoundSubscriber.h"
#include "server/GameSession.h"

#include <websocketpp/common/connection_hdl.hpp>

#include <string>

using ConnectionHdl = websocketpp::connection_hdl;

// One live match: owns GameSession + per-match RatingSubscriber.
class MatchRoom {
    int matchId_ = 0;
    GameSession session_;
    RatingSubscriber rating_;
    MoveLogSubscriber* moveLog_ = nullptr;
    SoundSubscriber* sound_ = nullptr;
    ConnectionHdl white_;
    ConnectionHdl black_;
    bool hasWhite_ = false;
    bool hasBlack_ = false;

public:
    MatchRoom(int matchId, UserService& users, MoveLogSubscriber* moveLog,
              SoundSubscriber* sound);
    ~MatchRoom();

    MatchRoom(const MatchRoom&) = delete;
    MatchRoom& operator=(const MatchRoom&) = delete;

    int matchId() const;
    GameSession& session();
    const GameSession& session() const;

    void seatPlayers(ConnectionHdl white, const std::string& whiteUser,
                     ConnectionHdl black, const std::string& blackUser);
    ConnectionHdl whiteHdl() const;
    ConnectionHdl blackHdl() const;
    bool hasWhite() const;
    bool hasBlack() const;
};

#endif
