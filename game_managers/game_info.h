#ifndef ROBOTS_GAME_INFO_H
#define ROBOTS_GAME_INFO_H

#include "../structures.h"
#include <map>
#include <cstdint>
#include <memory>
#include <mutex>
#include <ostream>
#include <unordered_set>
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


#endif //ROBOTS_GAME_INFO_H
