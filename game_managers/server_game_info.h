#ifndef ROBOTS_SERVER_GAME_INFO_H
#define ROBOTS_SERVER_GAME_INFO_H

#include "game_info.h"
#include "../structures.h"
#include <map>
#include <cstdint>
#include <memory>
#include <mutex>
#include <ostream>
#include <unordered_set>
#include <random>


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

#endif //ROBOTS_SERVER_GAME_INFO_H
