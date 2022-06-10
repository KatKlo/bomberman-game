#ifndef ROBOTS_CLIENT_GAME_INFO_H
#define ROBOTS_CLIENT_GAME_INFO_H

#include "game_info.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <ostream>
#include <unordered_set>
#include <random>


class ClientGameInfo : public GameInfo {
public:
    explicit ClientGameInfo(std::string player_name);

    DrawMessage::draw_message_optional_variant handle_server_message(ServerMessage::server_message_variant &msg);

    ClientMessage::client_message_optional_variant handle_GUI_message(InputMessage::input_message_variant &msg);

private:
    std::string player_name_;
    std::unordered_set<Position, Position::HashFunction> explosions;

    DrawMessage::draw_message_optional_variant generate_draw_message();
    DrawMessage::draw_message_optional_variant handle_hello(ServerMessage::Hello &msg);
    DrawMessage::draw_message_optional_variant handle_accepted_player(ServerMessage::AcceptedPlayer &msg);
    DrawMessage::draw_message_optional_variant handle_game_started(ServerMessage::GameStarted &msg);
    DrawMessage::draw_message_optional_variant handle_turn(ServerMessage::Turn &msg);
    DrawMessage::draw_message_optional_variant handle_game_ended();

    void handle_event(Event::event_message_variant &event);
    void handle_bomb_placed(Event::BombPlacedEvent &event);
    void handle_bomb_exploded(Event::BombExplodedEvent &event);
    void handle_player_moved(Event::PlayerMovedEvent &event);
    void handle_block_placed(Event::BlockPlacedEvent &event);

    void handle_explosion_for_position(int32_t x, int32_t y, bool &is_direction_ok) override;
};

#endif //ROBOTS_CLIENT_GAME_INFO_H
