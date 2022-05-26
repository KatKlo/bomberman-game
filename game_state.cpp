#include "game_state.h"
#include <algorithm>
#include <utility>
#include "logger.h"

GameInfo::GameInfo(std::string player_name) : player_name_(std::move(player_name)),
                                              basic_info(),
                                              players_count(),
                                              explosion_radius(),
                                              bomb_timer(),
                                              turn(),
                                              players(),
                                              bombs(),
                                              board(),
                                              explosions(),
                                              killed_players(),
                                              state(GameState::NotConnected) {}

DrawMessage::draw_message_optional_variant
GameInfo::handle_server_message(ServerMessage::server_message_variant &msg) {
    if (state == NotConnected && msg.index() != ServerMessage::HELLO) {
        return std::nullopt;
    }

    switch (msg.index()) {
        case ServerMessage::HELLO:
            return handle_hello(std::get<ServerMessage::Hello>(msg));
        case ServerMessage::ACCEPTED_PLAYER :
            return handle_accepted_player(std::get<ServerMessage::AcceptedPlayer>(msg));
        case ServerMessage::GAME_STARTED :
            return handle_game_started(std::get<ServerMessage::GameStarted>(msg));
        case ServerMessage::TURN :
            return handle_turn(std::get<ServerMessage::Turn>(msg));
        case ServerMessage::GAME_ENDED :
            return handle_game_ended();
        default:
            Logger::print_error("Internal problem with variant");
            return std::nullopt;
    }
}

ClientMessage::client_message_optional_variant
GameInfo::handle_GUI_message(InputMessage::input_message_variant &msg) {
    if (state == NotConnected) {
        return std::nullopt;
    } else if (state == GameState::Lobby) {
        return ClientMessage::Join{player_name_};
    } else {
        switch (msg.index()) {
            case InputMessage::PLACE_BOMB :
                return ClientMessage::PlaceBomb{};
            case InputMessage::PLACE_BLOCK :
                return ClientMessage::PlaceBlock{};
            case InputMessage::MOVE :
                return ClientMessage::Move{std::get<InputMessage::Move>(msg).direction};
            default:
                Logger::print_error("Internal problem with variant");
                return std::nullopt;
        }
    }
}

DrawMessage::draw_message_optional_variant GameInfo::generate_draw_message() {
    if (state == GameState::NotConnected) {
        return std::nullopt;
    } else if (state == GameState::Lobby) {
        return DrawMessage::Lobby(basic_info, players_count, explosion_radius, bomb_timer, players);
    } else {
        return DrawMessage::Game(basic_info, turn, players, bombs, board, explosions);
    }
}

DrawMessage::draw_message_optional_variant GameInfo::handle_hello(ServerMessage::Hello &msg) {
    basic_info = {msg.server_name, msg.size_x, msg.size_y, msg.game_length};
    players_count = msg.players_count;
    explosion_radius = msg.explosion_radius;
    bomb_timer = msg.bomb_timer;
    turn = 0;
    state = GameState::Lobby;

    board.resize(basic_info.size_x);
    for (auto &column: board) {
        for (int i = 0; i < basic_info.size_y; i++) {
            column.emplace_back(PositionType::Empty);
        }
    }

    return generate_draw_message();
}

DrawMessage::draw_message_optional_variant GameInfo::handle_accepted_player(ServerMessage::AcceptedPlayer &msg) {
    players.emplace(msg.id, PlayerInfo{msg.player, Position{0, 0}, 0});

    return generate_draw_message();
}

DrawMessage::draw_message_optional_variant GameInfo::handle_game_started(ServerMessage::GameStarted &msg) {
    state = GameState::Game;

    for (auto &it: msg.players) {
        players.emplace(it.first, PlayerInfo{it.second, Position{0, 0}, 0});
    }

    return std::nullopt;
}

DrawMessage::draw_message_optional_variant GameInfo::handle_turn(ServerMessage::Turn &msg) {
    if (state != GameState::Game) {
        return std::nullopt;
    }

    if (turn < msg.turn) {
        for (auto &it: bombs) {
            it.second.timer -= (msg.turn - turn);
        }

        turn = msg.turn;
    }

    killed_players.clear();
    explosions.clear();

    for (auto &it: msg.events) {
        handle_event(it);
    }

    for (auto id: killed_players) {
        players[id].score++;
    }

    for (auto &it: explosions) {
        board[it.x][it.y] = PositionType::Empty;
    }

    return generate_draw_message();
}

DrawMessage::draw_message_optional_variant GameInfo::handle_game_ended() {
    state = GameState::Lobby;
    players.clear();
    bombs.clear();

    for (auto &column: board) {
        for (auto &position: column) {
            position = PositionType::Empty;
        }
    }

    return generate_draw_message();
}

void GameInfo::handle_event(Event::event_message_variant &event) {
    switch (event.index()) {
        case Event::BOMB_PLACED :
            handle_bomb_placed(std::get<Event::BombPlacedEvent>(event));
            return;
        case Event::BOMB_EXPLODED :
            handle_bomb_exploded(std::get<Event::BombExplodedEvent>(event));
            return;
        case Event::PLAYER_MOVED :
            handle_player_moved(std::get<Event::PlayerMovedEvent>(event));
            return;
        case Event::BLOCK_PLACED :
            handle_block_placed(std::get<Event::BlockPlacedEvent>(event));
            return;
        default:
            Logger::print_error("Internal problem with variant");
            return;
    }
}

void GameInfo::handle_bomb_placed(Event::BombPlacedEvent &event) {
    bombs.emplace(event.id, Bomb{event.position, bomb_timer});
}

void GameInfo::handle_bomb_exploded(Event::BombExplodedEvent &event) {
    Position bomb_position = bombs[event.id].position;
    add_explosions_for_bomb(bomb_position);
    bombs.erase(event.id);

    for (auto &id: event.robots_destroyed) {
        killed_players.insert(id);
    }

    for (auto &block: event.blocks_destroyed) {
        explosions.insert(block);
    }
}

void GameInfo::handle_player_moved(Event::PlayerMovedEvent &event) {
    players[event.id].position = event.position;
}

void GameInfo::handle_block_placed(Event::BlockPlacedEvent &event) {
    board[event.position.x][event.position.y] = PositionType::Block;
}

void GameInfo::add_explosions_for_bomb(Position &bomb_position) {
    explosions.insert(bomb_position);

    uint16_t x = bomb_position.x;
    uint16_t y = bomb_position.y;
    while (board[x][y] != PositionType::Block && x > 0
           && x > bomb_position.x - explosion_radius) {
        x--;
        if (board[x][y] != PositionType::Block) {
            explosions.insert(Position{x, y});
        }
    }

    x = bomb_position.x;
    while (board[x][y] != PositionType::Block && x < basic_info.size_x - 1
           && x < bomb_position.x + explosion_radius) {
        x++;
        if (board[x][y] != PositionType::Block) {
            explosions.insert(Position{x, y});
        }
    }
    x = bomb_position.x;
    y = bomb_position.y;
    while (board[x][y] != PositionType::Block && y > 0
           && y > bomb_position.y - explosion_radius) {
        y--;
        if (board[x][y] != PositionType::Block) {
            explosions.insert(Position{x, y});
        }
    }

    y = bomb_position.y;
    while (board[x][y] != PositionType::Block && y < basic_info.size_y - 1
           && y < bomb_position.y + explosion_radius) {
        y++;
        if (board[x][y] != PositionType::Block) {
            explosions.insert(Position{x, y});
        }
    }
}