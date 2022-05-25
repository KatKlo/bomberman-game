#ifndef ROBOTS_STRUCTURES_H
#define ROBOTS_STRUCTURES_H

#include <iostream>
#include <string>
#include "utils.h"
#include <unordered_map>
#include <vector>
#include <optional>
#include <variant>

using message_id_t = uint8_t;
using event_id_t = uint8_t;
using player_id_t = uint8_t;
using score_t = uint32_t;
using bomb_id_t = uint32_t;

struct Address {
    std::string host;
    std::string port;

    friend std::istream &operator>>(std::istream &in, Address &address);

    friend std::ostream &operator<<(std::ostream &out, const Address &adr);
};

struct Player {
    std::string name;
    std::string address;
};

struct Position {
    uint16_t x;
    uint16_t y;
};

struct Bomb {
    Position position;
    uint16_t timer;
};

struct PlayerInfo {
    Player player;
    Position position;
    score_t score;
};

enum PositionType {
    Empty = 0,
    Block = 1,
    Explosion = 2
};

enum Direction : uint8_t {
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3
};

struct GameBasicInfo {
    std::string server_name;
    uint16_t size_x;
    uint16_t size_y;
    uint16_t game_length;
};

namespace ClientMessage {
    // client message id
    constexpr uint8_t JOIN = 0;
    constexpr uint8_t PLACE_BOMB = 1;
    constexpr uint8_t PLACE_BLOCK = 2;
    constexpr uint8_t MOVE = 3;

    struct Join {
        std::string name;
    };

    struct PlaceBomb {};

    struct PlaceBlock {};

    struct Move {
        Direction direction;
    };

    using client_message_variant = std::variant<
            Join,
            PlaceBomb,
            PlaceBlock,
            Move>;

    using client_message_optional_variant = std::optional<client_message_variant>;
}

namespace DrawMessage {
    // draw message id
    constexpr uint8_t LOBBY = 0;
    constexpr uint8_t GAME = 1;

    struct Lobby {
        Lobby(GameBasicInfo &info, uint8_t playersCount,
              uint16_t explosionRadius,
              uint16_t bombTimer,
              std::unordered_map<player_id_t, PlayerInfo> &p);

        std::string server_name;
        uint8_t players_count;
        uint16_t size_x;
        uint16_t size_y;
        uint16_t game_length;
        uint16_t explosion_radius;
        uint16_t bomb_timer;
        std::unordered_map<player_id_t, Player> players;
    };

    struct Game {
        Game(GameBasicInfo &info,
             uint16_t turn,
             std::unordered_map<player_id_t, PlayerInfo> &players_info,
             std::unordered_map<bomb_id_t, Bomb> &bombs_positions,
             std::vector<std::vector<PositionType>> &board);

        std::string server_name;
        uint16_t size_x;
        uint16_t size_y;
        uint16_t game_length;
        uint16_t turn;
        std::unordered_map<player_id_t, Player> players;
        std::unordered_map<player_id_t, Position> player_positions;
        std::vector<Position> blocks;
        std::vector<Bomb> bombs;
        std::vector<Position> explosions;
        std::unordered_map<player_id_t, score_t> scores;
    };

    using draw_message_variant = std::variant<Lobby, Game>;
    using draw_message_optional_variant = std::optional<draw_message_variant>;
}

namespace Event {
    // event id
    constexpr uint8_t BOMB_PLACED = 0;
    constexpr uint8_t BOMB_EXPLODED = 1;
    constexpr uint8_t PLAYER_MOVED = 2;
    constexpr uint8_t BLOCK_PLACED = 3;

    struct BombPlacedEvent {
        bomb_id_t id;
        Position position;
    };

    struct BombExplodedEvent {
        bomb_id_t id;
        std::vector<player_id_t> robots_destroyed;
        std::vector<Position> blocks_destroyed;
    };

    struct PlayerMovedEvent {
        player_id_t id;
        Position position;
    };

    struct BlockPlacedEvent {
        Position position;
    };

    using event_message_variant = std::variant<
            BombPlacedEvent,
            BombExplodedEvent,
            PlayerMovedEvent,
            BlockPlacedEvent>;
}

namespace ServerMessage {
    // server message id
    constexpr uint8_t HELLO = 0;
    constexpr uint8_t ACCEPTED_PLAYER = 1;
    constexpr uint8_t GAME_STARTED = 2;
    constexpr uint8_t TURN = 3;
    constexpr uint8_t GAME_ENDED = 4;

    struct Hello {
        std::string server_name;
        uint8_t players_count;
        uint16_t size_x;
        uint16_t size_y;
        uint16_t game_length;
        uint16_t explosion_radius;
        uint16_t bomb_timer;
    };

    struct AcceptedPlayer {
        player_id_t id;
        Player player;
    };

    struct GameStarted {
        std::unordered_map<player_id_t, Player> players;
    };

    struct Turn {
        uint16_t turn;
        std::vector<Event::event_message_variant> events;
    };

    struct GameEnded {
        std::unordered_map<player_id_t, score_t> scores;
    };

    using server_message_variant = std::variant<
            Hello,
            AcceptedPlayer,
            GameStarted,
            Turn,
            GameEnded>;

    using server_message_optional_variant = std::optional<server_message_variant>;
}

namespace InputMessage {
    // input message id
    constexpr uint8_t PLACE_BOMB = 0;
    constexpr uint8_t PLACE_BLOCK = 1;
    constexpr uint8_t MOVE = 2;

    struct PlaceBomb {};

    struct PlaceBlock {};

    struct Move {
        Direction direction;
    };

    using input_message_variant = std::variant<
            PlaceBomb,
            PlaceBlock,
            Move>;

    using input_message_optional_variant = std::optional<input_message_variant>;
}

#endif //ROBOTS_STRUCTURES_H
