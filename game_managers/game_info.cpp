#include "game_info.h"

using namespace std;

GameInfo::GameInfo(ServerParameters &params) : basic_info(params),
                                               players_count(params.get_players_count()),
                                               explosion_radius(params.get_explosion_radius()),
                                               bomb_timer(params.get_bomb_timer()),
                                               turn(0),
                                               players(),
                                               bombs(),
                                               blocks(),
                                               destroyed_robots() {}

bool GameInfo::is_position_on_board(int32_t x, int32_t y) const {
    return x >= 0 && x < static_cast<int32_t>(basic_info.size_x_)
           && y >= 0 && y < static_cast<int32_t>(basic_info.size_y_);
}

void GameInfo::clean_after_game() {
    state = GameState::Lobby;
    players.clear();
    bombs.clear();
    blocks.clear();
    turn = 0;
}

bool GameInfo::is_block_on_position(Position &position) {
    return blocks.find(position) != blocks.end();
}

void GameInfo::make_bomb_explosion(Position &bomb_position) {
    const uint8_t DIRECTIONS_NO = 4;
    bool is_bomb_on_empty = true;
    bool is_direction_ok[DIRECTIONS_NO];
    
    handle_explosion_for_position(bomb_position.x, bomb_position.y, is_bomb_on_empty);
    memset(is_direction_ok, is_bomb_on_empty, DIRECTIONS_NO);

    int32_t x = bomb_position.x;
    int32_t y = bomb_position.y;
    for (int32_t i = 1; i <= explosion_radius; i++) {
        handle_explosion_for_position(x, y + i, is_direction_ok[Direction::UP]);
        handle_explosion_for_position(x + i, y, is_direction_ok[Direction::RIGHT]);
        handle_explosion_for_position(x, y - i, is_direction_ok[Direction::DOWN]);
        handle_explosion_for_position(x - i, y, is_direction_ok[Direction::LEFT]);
    }
}

vector<player_id_t> GameInfo::get_players_on_position(Position &position) {
    vector<player_id_t> result;

    for (auto &[id, player]: players) {
        if (player.position == position) {
            result.emplace_back(id);
        }
    }

    return result;
}
