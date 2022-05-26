#include "structures.h"
#include "logger.h"
#include <exception>

DrawMessage::Lobby::Lobby(GameBasicInfo &info, uint8_t playersCount, uint16_t explosionRadius, uint16_t bombTimer,
                          std::unordered_map<player_id_t, PlayerInfo> &p) : server_name(info.server_name),
                                                                                             players_count(playersCount),
                                                                                             size_x(info.size_x),
                                                                                             size_y(info.size_y),
                                                                                             game_length(info.game_length),
                                                                                             explosion_radius(explosionRadius),
                                                                                             bomb_timer(bombTimer),
                                                                                             players() {
    for (auto &it: p) {
        players.emplace(it.first, it.second.player);
    }

}

DrawMessage::Game::Game(GameBasicInfo &info,
                        uint16_t turn,
                        std::unordered_map<player_id_t, PlayerInfo> &players_info,
                        std::unordered_map<bomb_id_t, Bomb> &bombs_positions,
                        std::vector<std::vector<PositionType>> &board) :
        server_name(info.server_name),
        size_x(info.size_x),
        size_y(info.size_y),
        game_length(info.game_length),
        turn(turn),
        players(),
        player_positions(),
        blocks(),
        bombs(),
        explosions(),
        scores() {
    for (auto &it: players_info) {
        players.emplace(it.first, it.second.player);
        player_positions.emplace(it.first, it.second.position);
        scores.emplace(it.first, it.second.score);
    }

    for (auto &it : bombs_positions) {
        bombs.emplace_back(it.second);
    }

    for (uint16_t i = 0 ; i < size_x; i++) {
        for (uint16_t j = 0; j < size_y; j++) {
            if (board[i][j] == PositionType::Block) {
                blocks.emplace_back(Position{i, j});
            }
            if (board[i][j] == PositionType::Explosion) {
                explosions.emplace_back(Position{i, j});
            }
        }
    }
}

std::ostream &DrawMessage::operator<<(std::ostream &os, const DrawMessage::Game &game) {
    os << "server_name: " << game.server_name << " size_x: " << game.size_x << " size_y: " << game.size_y
       << " game_length: " << game.game_length << " turn: " << game.turn << " players_size: " << game.players.size()
       << " player_positions_size: " << game.player_positions.size() << " blocks_size: " << game.blocks.size() << " bombs_size: " << game.bombs.size()
       << " explosions_size: " << game.explosions.size() << " scores_size: " << game.scores.size();
    return os;
}

Position::Position(uint16_t x, uint16_t y) : x(x), y(y) {}
