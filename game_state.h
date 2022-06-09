#ifndef ROBOTS_GAME_STATE_H
#define ROBOTS_GAME_STATE_H

#include <cstdint>
#include <memory>
#include "structures.h"
#include <mutex>
#include <ostream>
#include <unordered_set>
#include "parameters.h"
#include <random>

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
    std::map<player_id_t, PlayerInfo> players;
    std::map<bomb_id_t, Bomb> bombs;
    std::unordered_set<Position, Position::HashFunction> blocks;
    std::unordered_set<player_id_t> destroyed_robots;

    GameState state;

    GameInfo() = default;

    GameInfo(ServerParameters &params);

    bool is_position_on_board(int32_t x, int32_t y) const;
    bool is_block_on_position(Position &position);
    std::vector<player_id_t> get_players_on_position(Position &position);
    void clean_after_game();

    virtual void handle_explosion_for_position(int32_t x, int32_t y, bool &is_direction_ok) = 0;
    void make_bomb_explosion(Position &bomb_position);
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

    void handle_explosion_for_position(int32_t x, int32_t y, bool &is_direction_ok) override;
};

class ServerGameInfo : public GameInfo {
public:
    using start_game_messages = std::pair<ServerMessage::GameStarted, ServerMessage::Turn>;
    explicit ServerGameInfo(ServerParameters &params);

    bool is_enough_players() const;

    bool is_end_of_game() const;

    start_game_messages start_game();
    ServerMessage::GameEnded end_game();

    ServerMessage::Turn handle_turn(std::unordered_map<player_id_t, ClientMessage::client_message_variant> &msgs);
    std::optional<ServerMessage::AcceptedPlayer> handle_client_join_message(ClientMessage::Join &msg, std::string &&address);

private:
    uint16_t initial_blocks_;
    std::minstd_rand random_engine_;
    std::vector<Event::event_message_variant> events_;
    std::unordered_set<Position, Position::HashFunction> destroyed_blocks_;
    std::vector<Position> destroyed_blocks_in_explosion_;
    std::vector<player_id_t> destroyed_robots_in_explosion_;
    uint32_t next_bomb_id;

    void initialize_board();

    void handle_client_message_in_game(ClientMessage::client_message_variant &msg, PlayerInfo &player);

    void handle_place_bomb(Position &bomb_position);
    void handle_place_block(Position &block_position);
    void handle_move(ClientMessage::Move &msg, PlayerInfo &player);

    void handle_bomb_explosion(bomb_id_t bomb_id, Position &bomb_position);
    void handle_player_killed(PlayerInfo &player);

    Position get_random_position();
    void handle_explosion_for_position(int32_t x, int32_t y, bool &is_direction_ok) override;
};

#endif //ROBOTS_GAME_STATE_H
