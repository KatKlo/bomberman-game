#ifndef ROBOTS_GAME_INFO_H
#define ROBOTS_GAME_INFO_H

#include "../structures.h"
#include <map>
#include <unordered_set>

enum GameState {
    NotConnected,
    Lobby,
    Game
};

class GameInfo {
protected:
    GameBasicInfo basic_info;
    uint8_t players_count{};
    uint16_t explosion_radius{};
    uint16_t bomb_timer{};
    uint16_t turn{};
    std::map<player_id_t, PlayerInfo> players;
    std::map<bomb_id_t, Bomb> bombs;
    std::unordered_set<Position, Position::Hash> blocks;
    std::unordered_set<player_id_t> destroyed_robots;
    GameState state{NotConnected};

    GameInfo() = default;
    explicit GameInfo(ServerParameters &params);

    bool is_position_on_board(int32_t x, int32_t y) const;
    bool is_block_on_position(Position &position);

    std::vector<player_id_t> get_players_on_position(Position &position);

    void clean_after_game();

    void make_bomb_explosion(Position &bomb_position);

private:
    virtual void handle_explosion_for_position(int32_t x, int32_t y, bool &is_direction_ok) = 0;
};

#endif //ROBOTS_GAME_INFO_H