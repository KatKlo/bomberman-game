#ifndef ROBOTS_CLIENT_GAME_INFO_H
#define ROBOTS_CLIENT_GAME_INFO_H

#include "../structures.h"
#include "game_info.h"
#include <string>
#include <unordered_set>

class ClientGameInfo : public GameInfo {
public:
    explicit ClientGameInfo(std::string player_name);

    DrawMessage::draw_message_optional handle_server_message(ServerMessage::server_message &msg);
    ClientMessage::client_message_optional handle_GUI_message(InputMessage::input_message &msg);

private:
    std::string player_name_;
    std::unordered_set<Position, Position::Hash> explosions;

    DrawMessage::draw_message_optional generate_draw_message();

    DrawMessage::draw_message_optional handle_hello(ServerMessage::Hello &msg);
    DrawMessage::draw_message_optional handle_accepted_player(ServerMessage::AcceptedPlayer &msg);
    DrawMessage::draw_message_optional handle_game_started(ServerMessage::GameStarted &msg);
    DrawMessage::draw_message_optional handle_turn(ServerMessage::Turn &msg);
    DrawMessage::draw_message_optional handle_game_ended();

    void handle_event(Event::event_message &event);
    void handle_bomb_placed(Event::BombPlacedEvent &event);
    void handle_bomb_exploded(Event::BombExplodedEvent &event);
    void handle_player_moved(Event::PlayerMovedEvent &event);
    void handle_block_placed(Event::BlockPlacedEvent &event);

    void handle_explosion_for_position(int32_t x, int32_t y, bool &is_direction_ok) override;
};

#endif //ROBOTS_CLIENT_GAME_INFO_H
