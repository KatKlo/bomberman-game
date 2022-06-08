#ifndef ROBOTS_GAME_STATE_H
#define ROBOTS_GAME_STATE_H

#include <cstdint>
#include <memory>
#include "structures.h"
#include <mutex>
#include <ostream>
#include <unordered_set>
#include "parameters.h"

enum GameState {
    NotConnected,
    Lobby,
    Game
};

class GameInfo {
protected:
    GameBasicInfo basic_info;
    uint8_t players_count;
    uint16_t explosion_radius;
    uint16_t bomb_timer;
    uint16_t turn;
    std::unordered_map<player_id_t, PlayerInfo> players;
    std::unordered_map<bomb_id_t, Bomb> bombs;
    std::unordered_set<Position, Position::HashFunction> blocks;
    std::unordered_set<player_id_t> killed_players;

    GameState state;

    GameInfo() = default;

    GameInfo(ServerParameters &params);

    bool is_position_on_board(int32_t x, int32_t y) const;

    void clean_after_game();
};

class ClientGameInfo : public GameInfo {
public:
    explicit ClientGameInfo(std::string player_name);

    DrawMessage::draw_message_optional_variant handle_server_message(ServerMessage::server_message_variant &msg);

    ClientMessage::client_message_optional_variant handle_GUI_message(InputMessage::input_message_variant &msg);

private:
    std::string player_name_;
    std::unordered_set<Position, Position::HashFunction> explosions;

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

    void insert_explosion_if_possible(int32_t x, int32_t y, bool &is_direction_ok);
    void add_explosions_for_bomb(Position &bomb_position);
};

class ServerGameInfo : public GameInfo {
public:
    explicit ServerGameInfo(ServerParameters &params);

    bool is_enough_players() const;

    bool is_end_of_game() const;

    ServerMessage::GameStarted start_game();
    ServerMessage::GameEnded end_game();

    ServerMessage::server_message_variant handle_turn(std::unordered_map<player_id_t, ClientMessage::client_message_variant> &msgs);
    std::optional<ServerMessage::AcceptedPlayer> handle_client_join_message(ClientMessage::Join &msg, std::string &&address);

private:
    void handle_client_message_in_game(ClientMessage::client_message_variant &msg, player_id_t player_id);

    void handle_place_bomb(player_id_t player_id);
    void handle_place_block(player_id_t player_id);
    void handle_move(ClientMessage::Move &msg, player_id_t player_id);

};

#endif //ROBOTS_GAME_STATE_H
