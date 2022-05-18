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

namespace ClientMessages {
    // client_message_id
    // server
    constexpr uint8_t JOIN = 0;
    constexpr uint8_t PLACE_BOMB = 1;
    constexpr uint8_t PLACE_BLOCK = 2;
    constexpr uint8_t MOVE = 3;
    // gui
    constexpr uint8_t LOBBY = 0;
    constexpr uint8_t GAME = 1;



    struct JoinMessage {
        std::string name;
    };

    struct PlaceBombMessage {
    };

    struct PlaceBlockMessage {
    };

    struct MoveMessage {
        Direction direction;
    };

    using Client_server_message_variant = std::variant<
            JoinMessage,
            PlaceBombMessage,
            PlaceBlockMessage,
            MoveMessage>;

    using Client_server_message_optional_variant = std::optional<Client_server_message_variant>;

    struct LobbyMessage {
        LobbyMessage(GameBasicInfo &info, uint8_t playersCount,
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

    struct GameMessage {
        GameMessage(GameBasicInfo &info,
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

    using Client_GUI_message_variant = std::variant<LobbyMessage, GameMessage>;

    using Client_GUI_message_optional_variant = std::optional<Client_GUI_message_variant>;
}

namespace ServerMessage {
    // event_id
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

    // server_message_id
    constexpr uint8_t HELLO = 0;
    constexpr uint8_t ACCEPTED_PLAYER = 1;
    constexpr uint8_t GAME_STARTED = 2;
    constexpr uint8_t TURN = 3;
    constexpr uint8_t GAME_ENDED = 4;

    struct HelloMessage {
        std::string server_name;
        uint8_t players_count;
        uint16_t size_x;
        uint16_t size_y;
        uint16_t game_length;
        uint16_t explosion_radius;
        uint16_t bomb_timer;
    };

    struct AcceptedPlayerMessage {
        player_id_t id;
        Player player;
    };

    struct GameStartedMessage {
        std::unordered_map<player_id_t, Player> players;
    };

    struct TurnMessage {
        uint16_t turn;
        std::vector<event_message_variant> events;
    };
    struct GameEndedMessage {
        std::unordered_map<player_id_t, score_t> scores;
    };

    using Server_message_variant = std::variant<
            HelloMessage,
            AcceptedPlayerMessage,
            GameStartedMessage,
            TurnMessage,
            GameEndedMessage>;

    using Server_message_optional_variant = std::optional<Server_message_variant>;
}

namespace GUIMessages {
    // gui_message_id
    constexpr uint8_t PLACE_BOMB = 0;
    constexpr uint8_t PLACE_BLOCK = 1;
    constexpr uint8_t MOVE = 2;

    struct __attribute__((__packed__)) PlaceBombMessage {};

    struct __attribute__((__packed__)) PlaceBlockMessage {};

    struct __attribute__((__packed__)) MoveMessage {
        Direction direction;
    };

    using GUI_message_variant = std::variant<
            PlaceBombMessage,
            PlaceBlockMessage,
            MoveMessage>;

    using GUI_message_optional_variant = std::optional<GUI_message_variant>;
}

#endif //ROBOTS_STRUCTURES_H
