#include "structures.h"
#include "logger.h"

DrawMessage::Lobby::Lobby(GameBasicInfo &info,
                          uint8_t playersCount,
                          uint16_t explosionRadius,
                          uint16_t bombTimer,
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
                        std::unordered_set<Position, Position::HashFunction> &blocks_positions,
                        std::unordered_set<Position, Position::HashFunction> &explosions_positions) : server_name(info.server_name),
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

    for (auto &it: bombs_positions) {
        bombs.emplace_back(it.second);
    }

    for (auto &it: explosions_positions) {
        explosions.emplace_back(it);
    }

    for (auto &it: blocks_positions) {
        blocks.emplace_back(it);
    }
}

bool Position::operator==(const Position &rhs) const {
    return x == rhs.x && y == rhs.y;
}

size_t Position::HashFunction::operator()(const Position &pos) const {
    size_t hash_x = std::hash<uint16_t>()(pos.x);
    size_t hash_y = std::hash<uint16_t>()(pos.y) << 1;
    return hash_x ^ hash_y;
}
