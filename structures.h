#ifndef ROBOTS_STRUCTURES_H
#define ROBOTS_STRUCTURES_H

#include "parameters.h"
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

using message_id_t = uint8_t;
using event_id_t = uint8_t;
using player_id_t = uint8_t;
using score_t = uint32_t;
using bomb_id_t = uint32_t;
using board_coord_t = uint16_t;

struct Player {
    std::string name;
    std::string address;
};

struct Position {
    bool operator==(const Position &rhs) const;

    struct Hash {
        size_t operator()(const Position &pos) const;
    };

    board_coord_t x;
    board_coord_t y;
};

struct Bomb {
    Position position;
    uint16_t timer;
};

enum Direction : uint8_t {
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3,
};

struct PlayerInfo {
    player_id_t id{};
    Player player;
    Position position{};
    score_t score{};
};

struct GameBasicInfo {
    GameBasicInfo() = default;
    explicit GameBasicInfo(ServerParameters &params);
    GameBasicInfo(std::string &server_name, uint16_t size_x, uint16_t size_y, uint16_t game_length);

    std::string server_name_;
    board_coord_t size_x_{};
    board_coord_t size_y_{};
    uint16_t game_length_{};
};

namespace Event {
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

    using event_message = std::variant<BombPlacedEvent, BombExplodedEvent, PlayerMovedEvent, BlockPlacedEvent>;
}

namespace ClientMessage {
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
        Direction direction;
    };

    using client_message = std::variant<Join, PlaceBomb, PlaceBlock, Move>;
    using client_message_optional = std::optional<client_message>;
}

namespace DrawMessage {
    constexpr message_id_t LOBBY = 0;
    constexpr message_id_t GAME = 1;

    struct Lobby {
        Lobby(GameBasicInfo &info, uint8_t playersCount, uint16_t explosionRadius,
              uint16_t bombTimer, std::map<player_id_t, PlayerInfo> &p);

        std::string server_name_;
        uint8_t players_count_;
        board_coord_t size_x_;
        board_coord_t size_y;
        uint16_t game_length;
        uint16_t explosion_radius;
        uint16_t bomb_timer;
        std::unordered_map<player_id_t, Player> players;
    };

    struct Game {
        Game(GameBasicInfo &info, uint16_t turn, std::map<player_id_t, PlayerInfo> &players_info,
             std::map<bomb_id_t, Bomb> &bombs, std::unordered_set<Position, Position::Hash> &blocks,
             std::unordered_set<Position, Position::Hash> &explosions);

        std::string server_name;
        board_coord_t size_x;
        board_coord_t size_y;
        uint16_t game_length;
        uint16_t turn;
        std::unordered_map<player_id_t, Player> players;
        std::unordered_map<player_id_t, Position> player_positions;
        std::vector<Position> blocks;
        std::vector<Bomb> bombs_;
        std::vector<Position> explosions;
        std::unordered_map<player_id_t, score_t> scores;
    };

    using draw_message = std::variant<Lobby, Game>;
    using draw_message_optional = std::optional<draw_message>;
}

namespace ServerMessage {
    constexpr message_id_t HELLO = 0;
    constexpr message_id_t ACCEPTED_PLAYER = 1;
    constexpr message_id_t GAME_STARTED = 2;
    constexpr message_id_t TURN = 3;
    constexpr message_id_t GAME_ENDED = 4;

    struct Hello {
        Hello() = default;
        explicit Hello(ServerParameters &params);
        Hello(std::string &serverName, uint8_t playersCount, uint16_t sizeX, uint16_t sizeY,
              uint16_t gameLength, uint16_t explosionRadius, uint16_t bombTimer);

        std::string server_name;
        uint8_t players_count{};
        board_coord_t size_x{};
        board_coord_t size_y{};
        uint16_t game_length{};
        uint16_t explosion_radius{};
        uint16_t bomb_timer{};
    };

    struct AcceptedPlayer {
        player_id_t id{};
        Player player;
    };

    struct GameStarted {
        explicit GameStarted(const std::map<player_id_t, PlayerInfo> &players_info);
        explicit GameStarted(const std::unordered_map<player_id_t, Player> &players);

        std::unordered_map<player_id_t, Player> players;
    };

    struct Turn {
        uint16_t turn;
        std::vector<Event::event_message> events;
    };

    struct GameEnded {
        explicit GameEnded(const std::unordered_map<player_id_t, score_t> &scores);
        explicit GameEnded(const std::map<player_id_t, PlayerInfo> &players_info);

        std::unordered_map<player_id_t, score_t> scores;
    };

    using server_message = std::variant<Hello, AcceptedPlayer, GameStarted, Turn, GameEnded>;
}

namespace InputMessage {
    constexpr message_id_t PLACE_BOMB = 0;
    constexpr message_id_t PLACE_BLOCK = 1;
    constexpr message_id_t MOVE = 2;

    struct PlaceBomb {};

    struct PlaceBlock {};

    struct Move {
        Direction direction;
    };

    using input_message = std::variant<PlaceBomb, PlaceBlock, Move>;
}

#endif //ROBOTS_STRUCTURES_H