#pragma once

#include "bn_sprite_ptr.h"
#include "bn_fixed.h"

#include "log_orientation.h"

class board_game_state;
class falling_log {
private:
    bn::sprite_ptr sprite;
    log_orientation orientation{};
    int grid_x{};
    bn::fixed drop_speed{};

public:
    void tick(board_game_state &state);
    void activate(board_game_state &state, int new_grid_x, log_orientation new_orientation);
    void deactivate();
    bool is_active() const { return sprite.visible(); } 
    
    falling_log();
};