#include "game_state.h"
#include <algorithm>
#include <utility>
#include <cstring>
#include "logger.h"

GameInfo::GameInfo(ServerParameters &params) : basic_info(params),
                                               players_count(params.get_players_count()),
                                               explosion_radius(params.get_explosion_radius()),
                                               bomb_timer(params.get_bomb_timer()),
                                               turn(0),
                                               players(),
                                               bombs(),
                                               blocks(),
                                               destroyed_robots() {}

bool GameInfo::is_position_on_board(int32_t x, int32_t y) const {
    return x >= 0 && x < static_cast<int32_t>(basic_info.size_x) && y >= 0 &&
           y < static_cast<int32_t>(basic_info.size_y);
}

void GameInfo::clean_after_game() {
    state = GameState::Lobby;
    players.clear();
    bombs.clear();
    blocks.clear();
    turn = 0;
}

bool GameInfo::is_block_on_position(Position &position) {
    return blocks.find(position) != blocks.end();
}

void GameInfo::make_bomb_explosion(Position &bomb_position) {
    bool is_bomb_on_empty = true;
    handle_explosion_for_position(bomb_position.x, bomb_position.y, is_bomb_on_empty);
    bool is_direction_ok[4];
    std::memset(is_direction_ok, is_bomb_on_empty, 4);

    int32_t x = bomb_position.x;
    int32_t y = bomb_position.y;
    for (int32_t i = 1; i <= explosion_radius; i++) {
        handle_explosion_for_position(x, y + i, is_direction_ok[Direction::UP]);
        handle_explosion_for_position(x + i, y, is_direction_ok[Direction::RIGHT]);
        handle_explosion_for_position(x, y - i, is_direction_ok[Direction::DOWN]);
        handle_explosion_for_position(x - i, y, is_direction_ok[Direction::LEFT]);
    }
}

std::vector<player_id_t> GameInfo::get_players_on_position(Position &position) {
    std::vector<player_id_t> result;

    for (auto &[id, player]: players) {
        if (player.position == position) {
            result.emplace_back(id);
        }
    }

    return result;
}

ClientGameInfo::ClientGameInfo(std::string player_name) : GameInfo(), player_name_(std::move(player_name)) {
    this->state = GameState::NotConnected;
}

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
    basic_info = GameBasicInfo{msg.server_name, msg.size_x, msg.size_y, msg.game_length};
    players_count = msg.players_count;
    explosion_radius = msg.explosion_radius;
    bomb_timer = msg.bomb_timer;
    turn = 0;
    state = GameState::Lobby;

    return generate_draw_message();
}

DrawMessage::draw_message_optional_variant ClientGameInfo::handle_accepted_player(ServerMessage::AcceptedPlayer &msg) {
    players.emplace(msg.id, PlayerInfo{msg.id, msg.player, Position{0, 0}, 0});

    return generate_draw_message();
}

DrawMessage::draw_message_optional_variant ClientGameInfo::handle_game_started(ServerMessage::GameStarted &msg) {
    state = GameState::Game;

    for (auto &it: msg.players) {
        players.emplace(it.first, PlayerInfo{it.first, it.second, Position{0, 0}, 0});
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

DrawMessage::draw_message_optional_variant ClientGameInfo::handle_game_ended() {
    clean_after_game();
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

ServerGameInfo::ServerGameInfo(ServerParameters &params) : GameInfo(params),
                                                           initial_blocks_(params.get_initial_blocks()),
                                                           random_engine_(params.get_seed()),
                                                           events_(),
                                                           destroyed_blocks_(),
                                                           destroyed_blocks_in_explosion_(),
                                                           destroyed_robots_in_explosion_(),
                                                           next_bomb_id(0) {
    this->state = GameState::Lobby;
}

bool ServerGameInfo::is_enough_players() const {
    return players.size() >= players_count;
}

bool ServerGameInfo::is_end_of_game() const {
    return turn > basic_info.game_length;
}

ServerGameInfo::start_game_messages ServerGameInfo::start_game() {
    state = GameState::Game;
    initialize_board();
    return {ServerMessage::GameStarted{players}, ServerMessage::Turn{turn++, events_}};
}

ServerMessage::GameEnded ServerGameInfo::end_game() {
    ServerMessage::GameEnded result{players};
    clean_after_game();
    return result;
}

ServerMessage::Turn
ServerGameInfo::handle_turn(std::unordered_map<player_id_t, ClientMessage::client_message_variant> &msgs) {
    events_.clear();
    destroyed_robots.clear();
    destroyed_blocks_.clear();

    for(auto it = bombs.begin(); it != bombs.end(); ) {
        if (--(it->second.timer) == 0) {
            handle_bomb_explosion(it->first, it->second.position);
            it = bombs.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto &[id, player]: players) {
        if (destroyed_robots.find(id) == destroyed_robots.end()) {
            if (msgs.find(id) != msgs.end()) {
                handle_client_message_in_game(msgs[id], player);
            }
        } else {
            handle_player_killed(player);
        }
    }

    for (auto &it: destroyed_blocks_) {
        blocks.erase(it);
    }

    return {turn++, events_};
}

std::optional<ServerMessage::AcceptedPlayer>
ServerGameInfo::handle_client_join_message(ClientMessage::Join &msg, std::string &&address) {
    if (state != GameState::Lobby || players.size() > players_count) {
        return std::nullopt;
    }

    for (auto &it: players) {
        if (it.second.player.address == address) {
            return std::nullopt;
        }
    }

    Player new_player{msg.name, std::move(address)};
    player_id_t new_player_id = players.size();
    players.emplace(new_player_id, PlayerInfo{new_player_id, new_player, Position{0, 0}, 0});
    return ServerMessage::AcceptedPlayer{new_player_id, new_player};
}

void ServerGameInfo::handle_client_message_in_game(ClientMessage::client_message_variant &msg, PlayerInfo &player) {
    if (state != GameState::Game) {
        return;
    }

    switch (msg.index()) {
        case ClientMessage::JOIN :
            Logger::print_error("Join is ignored during game");
            return;
        case ClientMessage::PLACE_BOMB :
            handle_place_bomb(player.position);
            return;
        case ClientMessage::PLACE_BLOCK :
            handle_place_block(player.position);
            return;
        case ClientMessage::MOVE :
            handle_move(std::get<ClientMessage::Move>(msg), player);
            return;
        default:
            Logger::print_error("Internal problem with variant");
    }
}

void ServerGameInfo::initialize_board() {
    next_bomb_id = 0;
    events_.clear();

    for (auto &[id, player]: players) {
        player.position = get_random_position();
        events_.emplace_back(Event::PlayerMovedEvent{id, player.position});
    }

    for (uint16_t i = 0; i < initial_blocks_; i++) {
        Position block_position = get_random_position();
        events_.emplace_back(Event::BlockPlacedEvent{block_position});
        blocks.emplace(block_position);
    }
}

void ServerGameInfo::handle_place_bomb(Position &bomb_position) {
    bomb_id_t new_bomb_id = next_bomb_id++;
    bombs.emplace(new_bomb_id, Bomb{bomb_position, bomb_timer});
    events_.emplace_back(Event::BombPlacedEvent{new_bomb_id, bomb_position});
}

void ServerGameInfo::handle_place_block(Position &block_position) {
    if (is_block_on_position(block_position)) {
        return;
    }

    blocks.emplace(block_position);
    events_.emplace_back(Event::BlockPlacedEvent{block_position});
}

void ServerGameInfo::handle_move(ClientMessage::Move &msg, PlayerInfo &player) {
    Position player_position = player.position;
    int32_t x = (msg.direction == Direction::LEFT) ? (int32_t) player_position.x - 1 :
                (msg.direction == Direction::RIGHT) ? (int32_t) player_position.x + 1 : player_position.x;
    int32_t y = (msg.direction == Direction::UP) ? (int32_t) player_position.y + 1 :
                (msg.direction == Direction::DOWN) ? (int32_t) player_position.y - 1 : player_position.y;
    Position possible_new_position{static_cast<uint16_t>(x), static_cast<uint16_t>(y)};

    if (!is_position_on_board(x, y) || is_block_on_position(possible_new_position)) {
        return;
    }

    player.position = possible_new_position;
    events_.emplace_back(Event::PlayerMovedEvent{player.id, player.position});
}

void ServerGameInfo::handle_bomb_explosion(bomb_id_t bomb_id, Position &bomb_position) {
    destroyed_blocks_in_explosion_.clear();
    destroyed_robots_in_explosion_.clear();

    make_bomb_explosion(bomb_position);
    events_.emplace_back(Event::BombExplodedEvent{bomb_id, destroyed_robots_in_explosion_, destroyed_blocks_in_explosion_});
}

void ServerGameInfo::handle_player_killed(PlayerInfo &player) {
    player.score++;
    player.position = get_random_position();

    events_.emplace_back(Event::PlayerMovedEvent{player.id, player.position});
}

Position ServerGameInfo::get_random_position() {
    uint16_t x = random_engine_() % basic_info.size_x;
    uint16_t y = random_engine_() % basic_info.size_y;

    return {x, y};
}

void ServerGameInfo::handle_explosion_for_position(int32_t x, int32_t y, bool &is_direction_ok) {
    if (is_direction_ok && is_position_on_board(x, y)) {
        Position position = {static_cast<uint16_t>(x), static_cast<uint16_t>(y)};

        if (is_block_on_position(position)) {
            destroyed_blocks_in_explosion_.emplace_back(position);
            destroyed_blocks_.emplace(position);
            is_direction_ok = false;
        }

        std::vector<player_id_t> robots_to_destroy = get_players_on_position(position);
        for (auto &id: robots_to_destroy) {
            destroyed_robots_in_explosion_.emplace_back(id);
            destroyed_robots.emplace(id);
        }
    } else {
        is_direction_ok = false;
    }
}


