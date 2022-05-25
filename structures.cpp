#include "structures.h"
#include "utils.h"

std::istream &operator>>(std::istream &in, Address &address) {
    std::string token;
    in >> token;

    size_t delimiter_index = token.find_last_of(':');
    if (delimiter_index == std::string::npos || delimiter_index == token.size() - 1) {
        Logger::print_error("no port number in address \"", token, "\"");
    }

    address.port = token.substr(delimiter_index + 1, token.size() - 1 - delimiter_index);
    address.host = token.substr(0, delimiter_index);

    return in;
}

std::ostream &operator<<(std::ostream &out, const Address &adr) {
    return out << adr.host << ":" << adr.port;
}

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