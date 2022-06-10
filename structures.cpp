#include "structures.h"
#include "logger.h"

using namespace std;

GameBasicInfo::GameBasicInfo(ServerParameters &params) : server_name_(params.get_server_name()),
                                                         size_x_(params.get_size_x()),
                                                         size_y_(params.get_size_y()),
                                                         game_length_(params.get_game_length()) {}

GameBasicInfo::GameBasicInfo(string &server_name, uint16_t size_x, uint16_t size_y,
                             uint16_t game_length) : server_name_(server_name),
                                                     size_x_(size_x),
                                                     size_y_(size_y),
                                                     game_length_(game_length) {}

DrawMessage::Lobby::Lobby(GameBasicInfo &info, uint8_t playersCount,
                          uint16_t explosionRadius, uint16_t bombTimer,
                          map<player_id_t, PlayerInfo> &p) : server_name_(info.server_name_),
                                                             players_count_(playersCount),
                                                             size_x_(info.size_x_),
                                                             size_y(info.size_y_),
                                                             game_length(info.game_length_),
                                                             explosion_radius(explosionRadius),
                                                             bomb_timer(bombTimer),
                                                             players() {
    for (auto &it: p) {
        players.emplace(it.first, it.second.player);
    }
}

DrawMessage::Game::Game(GameBasicInfo &info, uint16_t turn, map<player_id_t, PlayerInfo> &players_info,
                        map<bomb_id_t, Bomb> &bombs, unordered_set<Position, Position::Hash> &blocks,
                        unordered_set<Position, Position::Hash> &explosions) : server_name(info.server_name_),
                                                                               size_x(info.size_x_),
                                                                               size_y(info.size_y_),
                                                                               game_length(info.game_length_),
                                                                               turn(turn),
                                                                               players(),
                                                                               player_positions(),
                                                                               blocks(blocks.begin(), blocks.end()),
                                                                               bombs_(),
                                                                               explosions(explosions.begin(), explosions.end()),
                                                                               scores() {
    for (auto &it: players_info) {
        players.emplace(it.first, it.second.player);
        player_positions.emplace(it.first, it.second.position);
        scores.emplace(it.first, it.second.score);
    }

    for (auto &it: bombs) {
        bombs_.emplace_back(it.second);
    }
}

bool Position::operator==(const Position &rhs) const {
    return x == rhs.x && y == rhs.y;
}

size_t Position::Hash::operator()(const Position &pos) const {
    size_t hash_x = hash<uint16_t>()(pos.x);
    size_t hash_y = hash<uint16_t>()(pos.y) << 1;
    return hash_x ^ hash_y;
}

ServerMessage::Hello::Hello(ServerParameters &params) : server_name(params.get_server_name()),
                                                        players_count(params.get_players_count()),
                                                        size_x(params.get_size_x()),
                                                        size_y(params.get_size_y()),
                                                        game_length(params.get_game_length()),
                                                        explosion_radius(params.get_explosion_radius()),
                                                        bomb_timer(params.get_bomb_timer()) {}

ServerMessage::Hello::Hello(string &serverName, uint8_t playersCount, uint16_t sizeX,
                            uint16_t sizeY, uint16_t gameLength, uint16_t explosionRadius,
                            uint16_t bombTimer) : server_name(serverName),
                                                  players_count(playersCount),
                                                  size_x(sizeX),
                                                  size_y(sizeY),
                                                  game_length(gameLength),
                                                  explosion_radius(explosionRadius),
                                                  bomb_timer(bombTimer) {}

ServerMessage::GameStarted::GameStarted(const map<player_id_t, PlayerInfo> &players_info) {
    for (auto &it: players_info) {
        players.emplace(it.first, it.second.player);
    }
}

ServerMessage::GameStarted::GameStarted(const unordered_map<player_id_t, Player> &players) : players(players) {}

ServerMessage::GameEnded::GameEnded(const unordered_map<player_id_t, score_t> &scores) : scores(scores) {}

ServerMessage::GameEnded::GameEnded(const map<player_id_t, PlayerInfo> &players_info) {
    for (auto &it: players_info) {
        scores.emplace(it.first, it.second.score);
    }
}

