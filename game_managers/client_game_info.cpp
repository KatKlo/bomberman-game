#include "client_game_info.h"
#include "../logger.h"

using namespace std;

ClientGameInfo::ClientGameInfo(string player_name) : GameInfo(),
                                                          player_name_(move(player_name)) {
    this->state = GameState::NotConnected;
}

DrawMessage::draw_message_optional ClientGameInfo::handle_server_message(ServerMessage::server_message &msg) {
    if (state == NotConnected && msg.index() != ServerMessage::HELLO) {
        return nullopt;
    }

    switch (msg.index()) {
        case ServerMessage::HELLO:
            return handle_hello(get<ServerMessage::Hello>(msg));
        case ServerMessage::ACCEPTED_PLAYER :
            return handle_accepted_player(get<ServerMessage::AcceptedPlayer>(msg));
        case ServerMessage::GAME_STARTED :
            return handle_game_started(get<ServerMessage::GameStarted>(msg));
        case ServerMessage::TURN :
            return handle_turn(get<ServerMessage::Turn>(msg));
        case ServerMessage::GAME_ENDED :
            return handle_game_ended();
        default:
            Logger::print_error("Internal problem with variant");
            return nullopt;
    }
}

ClientMessage::client_message_optional ClientGameInfo::handle_GUI_message(InputMessage::input_message &msg) {
    if (state == NotConnected) {
        return nullopt;
    } else if (state == GameState::Lobby) {
        return ClientMessage::Join{player_name_};
    } else {
        switch (msg.index()) {
            case InputMessage::PLACE_BOMB :
                return ClientMessage::PlaceBomb{};
            case InputMessage::PLACE_BLOCK :
                return ClientMessage::PlaceBlock{};
            case InputMessage::MOVE :
                return ClientMessage::Move{get<InputMessage::Move>(msg).direction};
            default:
                Logger::print_error("Internal problem with variant");
                return nullopt;
        }
    }
}

DrawMessage::draw_message_optional ClientGameInfo::generate_draw_message() {
    if (state == GameState::NotConnected) {
        return nullopt;
    } else if (state == GameState::Lobby) {
        return DrawMessage::Lobby(basic_info, players_count, explosion_radius, bomb_timer, players);
    } else {
        return DrawMessage::Game(basic_info, turn, players, bombs, blocks, explosions);
    }
}

DrawMessage::draw_message_optional ClientGameInfo::handle_hello(ServerMessage::Hello &msg) {
    basic_info = GameBasicInfo{msg.server_name, msg.size_x, msg.size_y, msg.game_length};
    players_count = msg.players_count;
    explosion_radius = msg.explosion_radius;
    bomb_timer = msg.bomb_timer;
    turn = 0;
    state = GameState::Lobby;

    return generate_draw_message();
}

DrawMessage::draw_message_optional ClientGameInfo::handle_accepted_player(ServerMessage::AcceptedPlayer &msg) {
    players.emplace(msg.id, PlayerInfo{msg.id, msg.player, Position{0, 0}, 0});

    return generate_draw_message();
}

DrawMessage::draw_message_optional ClientGameInfo::handle_game_started(ServerMessage::GameStarted &msg) {
    state = GameState::Game;

    for (auto &it: msg.players) {
        players.emplace(it.first, PlayerInfo{it.first, it.second, Position{0, 0}, 0});
    }

    return nullopt;
}

DrawMessage::draw_message_optional ClientGameInfo::handle_turn(ServerMessage::Turn &msg) {
    if (state != GameState::Game) {
        return nullopt;
    }

    if (turn < msg.turn) {
        uint16_t diff = msg.turn - turn;
        for (auto &it: bombs) {
            it.second.timer -= diff;
        }
    }

    turn = msg.turn;
    destroyed_robots.clear();
    explosions.clear();

    for (auto &it: msg.events) {
        handle_event(it);
    }

    for (auto id: destroyed_robots) {
        players[id].score++;
    }

    for (auto &it: explosions) {
        blocks.erase(it);
    }

    return generate_draw_message();
}

DrawMessage::draw_message_optional ClientGameInfo::handle_game_ended() {
    clean_after_game();
    return generate_draw_message();
}

void ClientGameInfo::handle_event(Event::event_message &event) {
    switch (event.index()) {
        case Event::BOMB_PLACED :
            handle_bomb_placed(get<Event::BombPlacedEvent>(event));
            return;
        case Event::BOMB_EXPLODED :
            handle_bomb_exploded(get<Event::BombExplodedEvent>(event));
            return;
        case Event::PLAYER_MOVED :
            handle_player_moved(get<Event::PlayerMovedEvent>(event));
            return;
        case Event::BLOCK_PLACED :
            handle_block_placed(get<Event::BlockPlacedEvent>(event));
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
    make_bomb_explosion(bomb_position);
    bombs.erase(event.id);

    for (auto &id: event.robots_destroyed) {
        destroyed_robots.insert(id);
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


void ClientGameInfo::handle_explosion_for_position(int32_t x, int32_t y, bool &is_direction_ok) {
    Position possible_position = {static_cast<uint16_t>(x), static_cast<uint16_t>(y)};

    if (is_direction_ok && is_position_on_board(x, y) && !is_block_on_position(possible_position)) {
        explosions.insert(possible_position);
    } else {
        is_direction_ok = false;
    }
}