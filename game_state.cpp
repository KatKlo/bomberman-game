#include "game_state.h"
#include <algorithm>

GameInfo::GameInfo(ServerMessage::HelloMessage &msg) : basic_info{msg.server_name, msg.size_x, msg.size_y, msg.game_length},
                                                       explosion_radius(msg.explosion_radius),
                                                       bomb_timer(msg.bomb_timer),
                                                       turn(0),
                                                       players(),
                                                       bombs(),
                                                       board() {
    board.resize(basic_info.size_x);
    for (auto &sth: board) {
        for (int i = 0; i < basic_info.size_y; i++) {
            sth.emplace_back(PositionType::Empty);
        }
    }
}

ClientMessages::Client_GUI_message_optional_variant
GameInfo::handle_server_message(ServerMessage::Server_message_variant &msg) {
    switch (msg.index()) {
        case ServerMessage::HELLO:
            return std::nullopt;
        case ServerMessage::ACCEPTED_PLAYER :
            add_accepted_player(std::get<ServerMessage::AcceptedPlayerMessage>(msg));
            return generate_message();
        case ServerMessage::GAME_STARTED :
            insert_players(std::get<ServerMessage::GameStartedMessage>(msg).players);
            state = GameState::Game;
            return std::nullopt;
        case ServerMessage::TURN :
            make_turn(std::get<ServerMessage::TurnMessage>(msg));
            return generate_message();
        case ServerMessage::GAME_ENDED :
            state = GameState::Lobby;
            return std::nullopt;
        default:
            Logger::print_error_and_exit("Internal problem with variant");
    }

    return std::nullopt;
}

ClientMessages::Client_GUI_message_optional_variant GameInfo::generate_message() {
    if (!changed) {
        return std::nullopt;
    } else if (state == GameState::Lobby) {
        return ClientMessages::LobbyMessage(basic_info, explosion_radius, bomb_timer, players);
    } else { // state == GameState::Game
        return ClientMessages::GameMessage(basic_info, turn, players, bombs, board);
    }
}

void GameInfo::add_accepted_player(ServerMessage::AcceptedPlayerMessage &msg) {
    changed = (state == GameState::Lobby);
    if (state == GameState::Lobby) {
        players.emplace(msg.id, PlayerInfo{msg.player, Position{0, 0}, 0});
    }
}

void GameInfo::make_turn(ServerMessage::TurnMessage &msg) {
    changed = (state == GameState::Game);
    if (state == GameState::Game) {
        if (turn < msg.turn) {
            for (uint16_t i = 0 ; i < basic_info.size_x; i++) {
                for (uint16_t j = 0; j < basic_info.size_y; j++) {
                    if (board[i][j] == PositionType::Explosion) {
                        board[i][j] = PositionType::Empty;
                    }
                }
            }

            for (auto &it : bombs) {
                it.second.timer -= (msg.turn - turn);
            }
        }

        for (auto &it : msg.events) {
            handle_event(it);
        }
    }
}

void GameInfo::handle_event(ServerMessage::event_message_variant &event) {
    switch (event.index()) {
        case ServerMessage::BOMB_PLACED : {
            auto sth = std::get<ServerMessage::BombPlacedEvent>(event);
            bombs.emplace(sth.id, Bomb{sth.position, bomb_timer});
        }
        case ServerMessage::BOMB_EXPLODED : {
            auto sth = std::get<ServerMessage::BombExplodedEvent>(event);
//            Position bomb_position = bombs[sth.id].position;
//          dodać eksplozje :)
            bombs.erase(sth.id);
//            nie chcemy tego robić dla każdej bomby:
//            for (auto id : sth.robots_destroyed) {
//                players[id].score++;
//            }
            for (auto &block : sth.blocks_destroyed) {
                board[block.x][block.y] = PositionType::Explosion;
            }
        }
        case ServerMessage::PLAYER_MOVED : {
            auto sth = std::get<ServerMessage::PlayerMovedEvent>(event);
            players[sth.id].position = sth.position;
        }
        case ServerMessage::BLOCK_PLACED : {
            auto sth = std::get<ServerMessage::BlockPlacedEvent>(event);
            board[sth.position.x][sth.position.y] = PositionType::Block;
        }
        default:
            Logger::print_error_and_exit("Internal problem with variant");
    }
}

void GameInfo::insert_players(std::unordered_map<player_id_t, Player> &playerss) {
    for (auto &it : playerss) {
        players.emplace(it.first, PlayerInfo{it.second, Position{0, 0}, 0});
    }

}