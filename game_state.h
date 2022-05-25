#ifndef ROBOTS_GAME_STATE_H
#define ROBOTS_GAME_STATE_H


#include <cstdint>
#include <memory>
#include "structures.h"

enum GameState {
    Lobby,
    Game
};

// TODO:
//  - zmienic konstruktor na tylko ze stringiem, dodać nowy stan "NotConnected" i handlowac hello na tym stanie żeby móc od razu zwraacac msg
//  - dodać struktury z msg dla gui żeby ich nie generowac za kazdym razem
//  - spróbować zmienic enum na struct i dodać template (zad 4 jnp1 + state patter jnp2) żeby podzielić analizowanie info
class GameInfo {
public:
    GameInfo(ServerMessage::Hello &msg, std::string &player_name);

    GameInfo() = default;

    DrawMessage::draw_message_optional_variant handle_server_message(ServerMessage::server_message_variant &msg);
    ClientMessage::client_message_optional_variant handle_GUI_message(InputMessage::input_message_variant &msg);


private:
    std::string player_name_;
    GameBasicInfo basic_info;
    uint8_t players_count;
    uint16_t explosion_radius;
    uint16_t bomb_timer;
    uint16_t turn;
    std::unordered_map<player_id_t, PlayerInfo> players;
    std::unordered_map<bomb_id_t, Bomb> bombs;
    std::vector<std::vector<PositionType>> board;

    bool changed;
    GameState state;

    DrawMessage::draw_message_optional_variant generate_message();
    void add_accepted_player(ServerMessage::AcceptedPlayer &msg);
    void make_turn(ServerMessage::Turn &msg);
    void handle_event(Event::event_message_variant &event);
    void insert_players(std::unordered_map<player_id_t, Player> &players);
    void change_to_lobby();
    void change_to_game();
    void reset_board();
};

#endif //ROBOTS_GAME_STATE_H
