#ifndef WEB_SOCKET_SERVER_H
#define WEB_SOCKET_SERVER_H

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#ifndef _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_STL_
#endif
// MinGW skips auto C++11 threads in websocketpp; force std::thread (no Boost).
#ifndef _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_CPP11_THREAD_
#endif

#include "auth/AuthController.h"
#include "auth/UserRepository.h"
#include "auth/UserService.h"
#include "bus/GameEvent.h"
#include "bus/MoveLogSubscriber.h"
#include "bus/SoundSubscriber.h"
#include "server/MatchQueue.h"
#include "server/MatchRoom.h"

#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using WsServer = websocketpp::server<websocketpp::config::asio>;
using ConnectionHdl = websocketpp::connection_hdl;

enum class ClientState {
    PendingAuth,
    Authenticated,
    Queued,
    Seated,
    Viewer
};

struct ClientConn {
    ClientState state = ClientState::PendingAuth;
    std::string username;
    int rating = 0;
    MatchQueue::PlayerId playerId = -1;
    int matchId = -1;
    char color = '?';
};

class WebSocketServer {
    static constexpr uint16_t kPort = 9002;
    static constexpr int kTickMs = 50;

    WsServer server_;
    UserRepository usersRepo_;
    UserService users_;
    AuthController auth_;
    MoveLogSubscriber moveLog_;
    SoundSubscriber sound_;

    std::map<ConnectionHdl, ClientConn, std::owner_less<ConnectionHdl>> clients_;
    std::unordered_map<MatchQueue::PlayerId, ConnectionHdl> playerToHdl_;
    std::map<int, std::unique_ptr<MatchRoom>> rooms_;
    MatchQueue matchQueue_;
    MatchQueue::PlayerId nextPlayerId_ = 1;
    int nextMatchId_ = 1;

    std::unique_ptr<asio::steady_timer> tickTimer_;
    bool running_ = false;

    void onOpen(ConnectionHdl hdl);
    void onClose(ConnectionHdl hdl);
    void onMessage(ConnectionHdl hdl, WsServer::message_ptr msg);
    void handleAuth(ConnectionHdl hdl, const std::string& line);
    void handlePlay(ConnectionHdl hdl);
    void tryMatch(ConnectionHdl joiner);
    void createMatch(ConnectionHdl white, ConnectionHdl black);
    int seatedPlayerCount() const;
    bool usernameAlreadyConnected(const std::string& username) const;
    void expireQueueEntries();
    void tearDownMatch(int matchId, ConnectionHdl exceptHdl);
    void scheduleTick();
    void onTick(const asio::error_code& ec);
    void broadcastToRoom(const MatchRoom& room, const std::string& payload);
    void sendJson(ConnectionHdl hdl, const std::string& payload);
    void sendSeatWelcome(ConnectionHdl hdl, MatchRoom& room, char color,
                         const std::vector<GameEvent>& events);

public:
    explicit WebSocketServer(const std::string& dbPath = "data/users.db");
    ~WebSocketServer();
    int run();
};

#endif
