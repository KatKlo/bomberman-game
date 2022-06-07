#include "game_state.h"
#include <algorithm>
#include <utility>
#include <cstring>
#include "logger.h"

ClientGameInfo::ClientGameInfo(std::string player_name) : player_name_(std::move(player_name)),
                                                          basic_info(),
                                                          players_count(),
                                                          explosion_radius(),
                                                          bomb_timer(),
                                                          turn(),
                                                          players(),
                                                          bombs(),
                                                          explosions(),
                                                          blocks(),
                                                          killed_players(),
                                                          state(GameState::NotConnected) {}

DrawMessage::draw_message_optional_variant
ClientGameInfo::handle_server_message(ServerMessage::server_message_variant &msg) {
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
ClientGameInfo::handle_GUI_message(InputMessage::input_message_variant &msg) {
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

DrawMessage::draw_message_optional_variant ClientGameInfo::generate_draw_message() {
    if (state == GameState::NotConnected) {
        return std::nullopt;
    } else if (state == GameState::Lobby) {
        return DrawMessage::Lobby(basic_info, players_count, explosion_radius, bomb_timer, players);
    } else {
        return DrawMessage::Game(basic_info, turn, players, bombs, blocks, explosions);
    }
}

DrawMessage::draw_message_optional_variant ClientGameInfo::handle_hello(ServerMessage::Hello &msg) {
    basic_info = {msg.server_name, msg.size_x, msg.size_y, msg.game_length};
    players_count = msg.players_count;
    explosion_radius = msg.explosion_radius;
    bomb_timer = msg.bomb_timer;
    turn = 0;
    state = GameState::Lobby;

    return generate_draw_message();
}

DrawMessage::draw_message_optional_variant ClientGameInfo::handle_accepted_player(ServerMessage::AcceptedPlayer &msg) {
    players.emplace(msg.id, PlayerInfo{msg.player, Position{0, 0}, 0});

    return generate_draw_message();
}

DrawMessage::draw_message_optional_variant ClientGameInfo::handle_game_started(ServerMessage::GameStarted &msg) {
    state = GameState::Game;

    for (auto &it: msg.players) {
        players.emplace(it.first, PlayerInfo{it.second, Position{0, 0}, 0});
    }

    return std::nullopt;
}

DrawMessage::draw_message_optional_variant ClientGameInfo::handle_turn(ServerMessage::Turn &msg) {
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
        blocks.erase(it);
    }

    return generate_draw_message();
}

DrawMessage::draw_message_optional_variant ClientGameInfo::handle_game_ended() {
    state = GameState::Lobby;
    players.clear();
    bombs.clear();
    blocks.clear();

    return generate_draw_message();
}

void ClientGameInfo::handle_event(Event::event_message_variant &event) {
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

void ClientGameInfo::handle_bomb_placed(Event::BombPlacedEvent &event) {
    bombs.emplace(event.id, Bomb{event.position, bomb_timer});
}

void ClientGameInfo::handle_bomb_exploded(Event::BombExplodedEvent &event) {
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

void ClientGameInfo::handle_player_moved(Event::PlayerMovedEvent &event) {
    players[event.id].position = event.position;
}

void ClientGameInfo::handle_block_placed(Event::BlockPlacedEvent &event) {
    blocks.emplace(event.position);
}

bool ClientGameInfo::is_position_on_board(int32_t x, int32_t y) const {
    return x >= 0 && x < static_cast<int32_t>(basic_info.size_x) && y >= 0 && y < static_cast<int32_t>(basic_info.size_y);
}

void ClientGameInfo::insert_explosion_if_possible(int32_t x, int32_t y, bool &is_direction_ok) {
    Position possible_position = {static_cast<uint16_t>(x), static_cast<uint16_t>(y)};

    if (is_direction_ok && is_position_on_board(x, y) && blocks.find(possible_position) == blocks.end()) {
        explosions.insert(possible_position);
    } else {
        is_direction_ok = false;
    }
}

void ClientGameInfo::add_explosions_for_bomb(Position &bomb_position) {
    explosions.insert(bomb_position);
    bool is_direction_ok[4] = {blocks.find(bomb_position) == blocks.end()};
    std::memset(is_direction_ok, blocks.find(bomb_position) == blocks.end(), 4);

    int32_t x = bomb_position.x;
    int32_t y = bomb_position.y;
    for (int32_t i = 1; i <= explosion_radius; i++) {
        insert_explosion_if_possible(x, y + i, is_direction_ok[0]);
        insert_explosion_if_possible(x + i, y, is_direction_ok[1]);
        insert_explosion_if_possible(x, y - i, is_direction_ok[2]);
        insert_explosion_if_possible(x - i, y, is_direction_ok[3]);
    }
}