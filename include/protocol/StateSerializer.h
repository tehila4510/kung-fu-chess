#ifndef PROTOCOL_STATE_SERIALIZER_H
#define PROTOCOL_STATE_SERIALIZER_H

#include "bus/GameEvent.h"
#include "engine/GameEngine.h"

#include <string>
#include <vector>

namespace protocol {

std::string serializeGameStateJson(const GameEngine& engine,
                                   const std::vector<GameEvent>& events,
                                   const std::string& status,
                                   const std::string& reason);

std::string serializeErrorJson(const std::string& reason);

std::string serializeWelcomeJson(char assignedColor);

std::string serializeAuthRequiredJson();

std::string serializeAuthOkJson(const std::string& username, int rating,
                                const std::string& role);

std::string serializeSearchingJson();

}  // namespace protocol

#endif
