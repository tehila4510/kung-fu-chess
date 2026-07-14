#ifndef PIECE_H
#define PIECE_H

#include "model/Position.h"
#include <optional>
#include <string>

struct Piece {
    static constexpr char NoColor = '.';
    static constexpr char NoKind  = '.';

    char kind  = NoKind;              // one of K Q R B N P
    char color = NoColor;             // 'w' or 'b'
    Position position{ -1, -1 };

    Piece() = default;
    Piece(char color, char kind, Position position);

    static std::optional<Piece> tryFromToken(const std::string& token, Position at);
    static Piece fromToken(const std::string& token, Position at);

    std::string toToken() const;

    bool isEmpty() const;
    bool isWhite() const;
    bool isBlack() const;

    bool sameColorAs(const Piece& other) const;
    bool isEnemyOf(const Piece& other) const;

    char opponentColor() const;

    bool operator==(const Piece& o) const;
    bool operator!=(const Piece& o) const;
};

#endif
