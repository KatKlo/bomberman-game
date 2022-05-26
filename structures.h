#ifndef ROBOTS_STRUCTURES_H
#define ROBOTS_STRUCTURES_H

#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <optional>
#include <variant>

// Module for all structures needed for transferring and storing data

using message_id_t = uint8_t;
using event_id_t = uint8_t;
using player_id_t = uint8_t;
using score_t = uint32_t;
using bomb_id_t = uint32_t;
using direction_t = uint8_t;

struct Player {
    std::string name;
    std::string address;
};

struct Position {
    bool operator<(const Position &rhs) const;

    uint16_t x;
    uint16_t y;
};

struct Bomb {
    Position position;
    uint16_t timer;
};

struct PlayerInfo {
    Player player;
    Position position{};
    score_t score{};
};

enum PositionType {
    Empty = 0,
    Block = 1,
};

struct GameBasicInfo {
    GameBasicInfo() = default;

    std::string server_name;
    uint16_t size_x{};
    uint16_t size_y{};
    uint16_t game_length{};
};

namespace Event {
    // event id
    constexpr event_id_t BOMB_PLACED = 0;
    constexpr event_id_t BOMB_EXPLODED = 1;
    constexpr event_id_t PLAYER_MOVED = 2;
    constexpr event_id_t BLOCK_PLACED = 3;

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

namespace ClientMessage {
    // client message id
    constexpr message_id_t JOIN = 0;
    constexpr message_id_t PLACE_BOMB = 1;
    constexpr message_id_t PLACE_BLOCK = 2;
    constexpr message_id_t MOVE = 3;

    struct Join {
        std::string name;
    };

    struct PlaceBomb {};

    struct PlaceBlock {};

    struct Move {
        direction_t direction;
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
    constexpr message_id_t LOBBY = 0;
    constexpr message_id_t GAME = 1;

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
             std::vector<std::vector<PositionType>> &board,
             std::set<Position> &explosions);

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

namespace ServerMessage {
    // server message id
    constexpr message_id_t HELLO = 0;
    constexpr message_id_t ACCEPTED_PLAYER = 1;
    constexpr message_id_t GAME_STARTED = 2;
    constexpr message_id_t TURN = 3;
    constexpr message_id_t GAME_ENDED = 4;

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
        player_id_t id{};
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
}

namespace InputMessage {
    // input message id
    constexpr message_id_t PLACE_BOMB = 0;
    constexpr message_id_t PLACE_BLOCK = 1;
    constexpr message_id_t MOVE = 2;

    struct PlaceBomb {};

    struct PlaceBlock {};

    struct Move {
        direction_t direction;
    };

    using input_message_variant = std::variant<
            PlaceBomb,
            PlaceBlock,
            Move>;
}

#endif //ROBOTS_STRUCTURES_H
