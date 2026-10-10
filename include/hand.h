#pragma once

#include "bn_sprite_ptr.h"

#include "log_orientation.h"

enum class move_direction {
    LEFT = -1,
    NONE = 0,
    RIGHT = 1
};

class board_game_state;
class hand {
private:
    bn::sprite_ptr sprite;
    int column{};
    int row{};

    bool holding_log{};
    log_orientation held_log_orientation{};

    move_direction held_direction{};
    int hold_repeat_time{};

    bool can_enter_axe_mode(board_game_state &state) const;

public:
    void tick(board_game_state &state);

    hand(board_game_state &state);
};