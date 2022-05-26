#include "game_state.h"
#include <algorithm>
#include <utility>
#include <unordered_set>
#include "logger.h"

DrawMessage::draw_message_optional_variant
GameInfo::handle_server_message(ServerMessage::server_message_variant &msg) {
    switch (msg.index()) {
        case ServerMessage::HELLO:
            make_hello(std::get<ServerMessage::Hello>(msg));
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
//            Logger::print_info("after turn: ", *this);
            return generate_message();
        case ServerMessage::GAME_ENDED :
            change_to_lobby();
            reset_board();
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
        auto res = DrawMessage::Game(basic_info, turn, players, bombs, board);
//        Logger::print_info("new message: ", res);
        return res;
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

//        Logger::print_info("before handling events: ", *this);


        std::unordered_set<player_id_t> killed_players;
        std::vector<Position> additional_explosions;
        for (auto &it: msg.events) {
            handle_event(it, killed_players, additional_explosions);
//            Logger::print_info("after event: ", *this);
        }
        turn = msg.turn;

        for (auto id: killed_players) {
            players[id].score++;
        }

        for (auto pos: additional_explosions) {
            board[pos.x][pos.y] = PositionType::Explosion;
        }
    }
}

void GameInfo::add_explosions(Position &bomb_position) {
    uint16_t x = bomb_position.x, y = bomb_position.y;
    if (board[x][y] == PositionType::Block) {
        return;
    }

    while (x > 0 && x > bomb_position.x - explosion_radius) {
        x--;
        if (board[x][y] != PositionType::Block) {
            board[x][y] = PositionType::Explosion;
        } else {
            break;
        }
    }

    x = bomb_position.x;
    while (x < basic_info.size_x - 1 && x < bomb_position.x + explosion_radius) {
        x++;
        if (board[x][y] != PositionType::Block) {
            board[x][y] = PositionType::Explosion;
        } else {
            break;
        }
    }
    x = bomb_position.x;
    y = bomb_position.y;

    while (y > 0 && y > bomb_position.y - explosion_radius) {
        y--;
        if (board[x][y] != PositionType::Block) {
            board[x][y] = PositionType::Explosion;
        } else {
            break;
        }
    }

    y = bomb_position.y;
    while (y < basic_info.size_y - 1 && y < bomb_position.y + explosion_radius) {
        y++;
        if (board[x][y] != PositionType::Block) {
            board[x][y] = PositionType::Explosion;
        } else {
            break;
        }
    }
}

void GameInfo::handle_event(Event::event_message_variant &event, std::unordered_set<player_id_t> &killed_players, std::vector<Position> &additional_explosions) {
    switch (event.index()) {
        case Event::BOMB_PLACED : {
            auto sth = std::get<Event::BombPlacedEvent>(event);
            bombs.emplace(sth.id, Bomb{sth.position, bomb_timer});
            break;
        }
        case Event::BOMB_EXPLODED : {
            auto sth = std::get<Event::BombExplodedEvent>(event);
            Position bomb_position = bombs[sth.id].position;
            add_explosions(bomb_position);
            additional_explosions.push_back(bomb_position);
            bombs.erase(sth.id);
            for (auto id: sth.robots_destroyed) {
                killed_players.insert(id);
            }
            for (auto &block: sth.blocks_destroyed) {
                additional_explosions.push_back(block);
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

GameInfo::GameInfo(std::string player_name) :
        player_name_(std::move(player_name)),
        basic_info{std::string(), uint16_t(), uint16_t(), uint16_t()},
        players_count(),
        explosion_radius(),
        bomb_timer(),
        turn(),
        players(),
        bombs(),
        board(),
        changed(false),
        state(GameState::NotConnected) {}

void GameInfo::make_hello(ServerMessage::Hello &msg) {
    basic_info = {msg.server_name, msg.size_x, msg.size_y, msg.game_length};
    players_count = msg.players_count;
    explosion_radius = msg.explosion_radius;
    bomb_timer = msg.bomb_timer;
    turn = 0;
    changed = true;
    state = GameState::Lobby;
    board.resize(basic_info.size_x);
    for (auto &sth: board) {
        for (int i = 0; i < basic_info.size_y; i++) {
            sth.emplace_back(PositionType::Empty);
        }
    }
}

bool GameInfo::is_connected() {
    return state != GameState::NotConnected;
}

GameInfo::GameInfo() : player_name_(),
                       basic_info{std::string(), uint16_t(), uint16_t(), uint16_t()},
                       players_count(),
                       explosion_radius(),
                       bomb_timer(),
                       turn(),
                       players(),
                       bombs(),
                       board(),
                       changed(false),
                       state(GameState::NotConnected) {}

std::ostream &operator<<(std::ostream &os, const GameInfo &info) {
    os << " players_size: " << info.players.size() << " bombs_size: " << info.bombs.size() << " board_size: "
       << info.board.size();
    return os;
}
