#include "structures.h"
#include "logger.h"

GameBasicInfo::GameBasicInfo(ServerParameters &params) : server_name(params.get_server_name()),
                                                         size_x(params.get_size_x()),
                                                         size_y(params.get_size_y()),
                                                         game_length(params.get_game_length()) {}

GameBasicInfo::GameBasicInfo(const std::string &server_name, uint16_t size_x, uint16_t size_y, uint16_t game_length)
        : server_name(server_name), size_x(size_x), size_y(size_y), game_length(game_length) {}

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
                        std::unordered_set<Position, Position::HashFunction> &explosions_positions) : server_name(
        info.server_name),
                                                                                                      size_x(info.size_x),
                                                                                                      size_y(info.size_y),
                                                                                                      game_length(
                                                                                                              info.game_length),
                                                                                                      turn(turn),
                                                                                                      players(),
                                                                                                      player_positions(),
                                                                                                      blocks(blocks_positions.begin(),
                                                                                                             blocks_positions.end()),
                                                                                                      bombs(),
                                                                                                      explosions(
                                                                                                              explosions_positions.begin(),
                                                                                                              explosions_positions.end()),
                                                                                                      scores() {
    for (auto &it: players_info) {
        players.emplace(it.first, it.second.player);
        player_positions.emplace(it.first, it.second.position);
        scores.emplace(it.first, it.second.score);
    }

    for (auto &it: bombs_positions) {
        bombs.emplace_back(it.second);
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

ServerMessage::Hello::Hello(ServerParameters &params) : server_name(params.get_server_name()),
                                                        players_count(params.get_players_count()),
                                                        size_x(params.get_size_x()),
                                                        size_y(params.get_size_y()),
                                                        game_length(params.get_game_length()),
                                                        explosion_radius(params.get_explosion_radius()),
                                                        bomb_timer(params.get_bomb_timer()) {}

ServerMessage::Hello::Hello(const std::string &serverName, uint8_t playersCount,
                            uint16_t sizeX, uint16_t sizeY, uint16_t gameLength, uint16_t
                            explosionRadius, uint16_t bombTimer) : server_name(serverName),
                                                                   players_count(playersCount),
                                                                   size_x(sizeX),
                                                                   size_y(sizeY),
                                                                   game_length(gameLength),
                                                                   explosion_radius(explosionRadius),
                                                                   bomb_timer(bombTimer) {}
