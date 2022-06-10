#include "server_game_info.h"
#include "../logger.h"

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
    if (state != GameState::Lobby || players.size() >= players_count) {
        return std::nullopt;
    }

    for (auto &it: players) {
        if (it.second.player.address == address) {
            return std::nullopt;
        }
    }

    Player new_player{msg.name, std::move(address)};
    player_id_t new_player_id = static_cast<uint8_t>(players.size());
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
        auto [it, is_inserted] = blocks.emplace(block_position);
        if (is_inserted) {
            events_.emplace_back(Event::BlockPlacedEvent{block_position});
        }
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
    uint16_t x = static_cast<uint16_t>(random_engine_() % basic_info.size_x);
    uint16_t y = static_cast<uint16_t>(random_engine_() % basic_info.size_y);

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
