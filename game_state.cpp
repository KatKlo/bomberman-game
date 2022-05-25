#include "game_state.h"
#include <algorithm>

GameInfo::GameInfo(ServerMessage::Hello &msg, std::string &player_name) :
        player_name_(player_name),
        basic_info{msg.server_name, msg.size_x, msg.size_y, msg.game_length},
        players_count(msg.players_count),
        explosion_radius(msg.explosion_radius),
        bomb_timer(msg.bomb_timer),
        turn(0),
        players(),
        bombs(),
        board(),
        changed(true),
        state(GameState::Lobby){
    board.resize(basic_info.size_x);
    for (auto &sth: board) {
        for (int i = 0; i < basic_info.size_y; i++) {
            sth.emplace_back(PositionType::Empty);
        }
    }
}

DrawMessage::draw_message_optional_variant
GameInfo::handle_server_message(ServerMessage::server_message_variant &msg) {
    switch (msg.index()) {
        case ServerMessage::HELLO:
            return generate_message();
        case ServerMessage::ACCEPTED_PLAYER :
            add_accepted_player(std::get<ServerMessage::AcceptedPlayer>(msg));
            return generate_message();
        case ServerMessage::GAME_STARTED :
            change_to_game();
            insert_players(std::get<ServerMessage::GameStarted>(msg).players);
            return std::nullopt;
        case ServerMessage::TURN :
            make_turn(std::get<ServerMessage::Turn>(msg));
            return generate_message();
        case ServerMessage::GAME_ENDED :
            change_to_lobby();
            return generate_message();
        default:
            Logger::print_error("Internal problem with variant");
    }

    return std::nullopt;
}

DrawMessage::draw_message_optional_variant GameInfo::generate_message() {
    if (!changed) {
        return std::nullopt;
    } else if (state == GameState::Lobby) {
        return DrawMessage::Lobby(basic_info, players_count, explosion_radius, bomb_timer, players);
    } else { // state == GameState::Game
        return DrawMessage::Game(basic_info, turn, players, bombs, board);
    }
}

void GameInfo::add_accepted_player(ServerMessage::AcceptedPlayer &msg) {
    changed = (state == GameState::Lobby);
    if (state == GameState::Lobby) {
        players.emplace(msg.id, PlayerInfo{msg.player, Position{0, 0}, 0});
    }
}

void GameInfo::make_turn(ServerMessage::Turn &msg) {
    changed = (state == GameState::Game);
    if (state == GameState::Game) {
        if (turn < msg.turn) {
            for (uint16_t i = 0; i < basic_info.size_x; i++) {
                for (uint16_t j = 0; j < basic_info.size_y; j++) {
                    if (board[i][j] == PositionType::Explosion) {
                        board[i][j] = PositionType::Empty;
                    }
                }
            }

            for (auto &it: bombs) {
                it.second.timer -= (msg.turn - turn);
            }
        }

        for (auto &it: msg.events) {
            handle_event(it);
        }
    }
}

void GameInfo::handle_event(Event::event_message_variant &event) {
    switch (event.index()) {
        case Event::BOMB_PLACED : {
            auto sth = std::get<Event::BombPlacedEvent>(event);
            bombs.emplace(sth.id, Bomb{sth.position, bomb_timer});
            break;
        }
        case Event::BOMB_EXPLODED : {
            auto sth = std::get<Event::BombExplodedEvent>(event);
//            Position bomb_position = bombs[sth.id].position;
//          dodać eksplozje :)
            bombs.erase(sth.id);
//            nie chcemy tego robić dla każdej bomby:
            for (auto id: sth.robots_destroyed) {
                players[id].score++;
            }
            for (auto &block: sth.blocks_destroyed) {
                board[block.x][block.y] = PositionType::Explosion;
            }
            break;
        }
        case Event::PLAYER_MOVED : {
            auto sth = std::get<Event::PlayerMovedEvent>(event);
            players[sth.id].position = sth.position;
            break;
        }
        case Event::BLOCK_PLACED : {
            auto sth = std::get<Event::BlockPlacedEvent>(event);
            board[sth.position.x][sth.position.y] = PositionType::Block;
            break;
        }
        default:
            Logger::print_error("Internal problem with variant");
    }
}

void GameInfo::insert_players(std::unordered_map<player_id_t, Player> &playerss) {
    for (auto &it: playerss) {
        players.emplace(it.first, PlayerInfo{it.second, Position{0, 0}, 0});
    }

}

ClientMessage::client_message_optional_variant
GameInfo::handle_GUI_message(InputMessage::input_message_variant &msg) {
    if (state == GameState::Lobby) {
        return ClientMessage::Join{player_name_};
    } else {
        switch (msg.index()) {
            case InputMessage::PLACE_BOMB : {
                return ClientMessage::PlaceBomb{};
            }
            case InputMessage::PLACE_BLOCK : {
                return ClientMessage::PlaceBlock{};
            }
            case InputMessage::MOVE : {
                auto sth = std::get<InputMessage::Move>(msg);
                return ClientMessage::Move{sth.direction};
            }
            default: {
                Logger::print_error("Internal problem with variant");
                return std::nullopt;
            }
        }
    }
}

void GameInfo::change_to_game() {
    state = GameState::Game;
    changed = true;
    reset_board();
}

void GameInfo::change_to_lobby() {
    state = GameState::Lobby;
    changed = true;
}

void GameInfo::reset_board() {
    for (auto &sth: board) {
        for (auto &sth2: sth) {
            sth2 = PositionType::Empty;
        }
    }
    players.clear();
    bombs.clear();
}
