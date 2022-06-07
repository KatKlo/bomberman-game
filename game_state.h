#ifndef ROBOTS_GAME_STATE_H
#define ROBOTS_GAME_STATE_H


#include <cstdint>
#include <memory>
#include "structures.h"
#include <mutex>
#include <ostream>
#include <unordered_set>

enum GameState {
    NotConnected,
    Lobby,
    Game
};

class ClientGameInfo {
public:
    explicit ClientGameInfo(std::string player_name);

    DrawMessage::draw_message_optional_variant handle_server_message(ServerMessage::server_message_variant &msg);
    ClientMessage::client_message_optional_variant handle_GUI_message(InputMessage::input_message_variant &msg);

private:
    std::string player_name_;
    GameBasicInfo basic_info;
    uint8_t players_count;
    uint16_t explosion_radius;
    uint16_t bomb_timer;
    uint16_t turn;
    std::unordered_map<player_id_t, PlayerInfo> players;
    std::unordered_map<bomb_id_t, Bomb> bombs;
    std::unordered_set<Position, Position::HashFunction> explosions;
    std::unordered_set<Position, Position::HashFunction> blocks;
    std::unordered_set<player_id_t> killed_players;

    GameState state;

    DrawMessage::draw_message_optional_variant generate_draw_message();

    DrawMessage::draw_message_optional_variant handle_hello(ServerMessage::Hello &msg);
    DrawMessage::draw_message_optional_variant handle_accepted_player(ServerMessage::AcceptedPlayer &msg);
    DrawMessage::draw_message_optional_variant handle_game_started(ServerMessage::GameStarted &msg);
    DrawMessage::draw_message_optional_variant handle_turn(ServerMessage::Turn &msg);
    DrawMessage::draw_message_optional_variant handle_game_ended();

    void handle_event(Event::event_message_variant &event);

    void handle_bomb_placed(Event::BombPlacedEvent &event);
    void handle_bomb_exploded(Event::BombExplodedEvent &event);
    void handle_player_moved(Event::PlayerMovedEvent &event);
    void handle_block_placed(Event::BlockPlacedEvent &event);

    bool is_position_on_board(int32_t x, int32_t y) const;
    void insert_explosion_if_possible(int32_t x, int32_t y, bool &is_direction_ok);
    void add_explosions_for_bomb(Position &bomb_position);
};

#endif //ROBOTS_GAME_STATE_H
