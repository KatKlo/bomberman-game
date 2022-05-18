#ifndef ROBOTS_GAME_STATE_H
#define ROBOTS_GAME_STATE_H


#include <cstdint>
#include <memory>
#include "structures.h"

enum GameState {
    Lobby,
    Game
};

class GameInfo {
public:
    GameInfo(ServerMessage::HelloMessage &msg, std::string &player_name);

    GameInfo() = default;

    ClientMessages::Client_GUI_message_optional_variant handle_server_message(ServerMessage::Server_message_variant &msg);
    ClientMessages::Client_server_message_optional_variant handle_GUI_message(GUIMessages::GUI_message_variant &msg);
    bool is_in_lobby();

private:
    std::string player_name_;
    GameBasicInfo basic_info;
    uint16_t explosion_radius;
    uint16_t bomb_timer;
    uint16_t turn;
    std::unordered_map<player_id_t, PlayerInfo> players;
    std::unordered_map<bomb_id_t, Bomb> bombs;
    std::vector<std::vector<PositionType>> board;

    bool changed;
    GameState state;

    ClientMessages::Client_GUI_message_optional_variant generate_message();
    void add_accepted_player(ServerMessage::AcceptedPlayerMessage &msg);
    void make_turn(ServerMessage::TurnMessage &msg);
    void handle_event(ServerMessage::event_message_variant &event);
    void insert_players(std::unordered_map<player_id_t, Player> &players);
};

#endif //ROBOTS_GAME_STATE_H
