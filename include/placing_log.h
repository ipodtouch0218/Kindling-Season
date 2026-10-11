#pragma once

#include "bn_array.h"
#include "bn_fixed_point.h"
#include "bn_point.h"
#include "bn_sprite_ptr.h"

#include "enums.h"

class board_game_state;
class placing_log {
private:
    bn::array<bn::sprite_ptr, 4> sprites;
    bn::array<log_orientation, 4> orientations;

    bn::fixed fall_speed;
    bn::fixed fall_progress;
    bn::point grid_position;

    bool _active{};

    void place_onto_board(board_game_state &state);
    bool can_rotate(const board_game_state &state) const;
    bool can_move_into(bn::point grid_position, const board_game_state &state) const;
    void rotate_cw();
    void rotate_ccw();

public:
    void tick(board_game_state &state);
    void spawn_at_top(board_game_state &state, bn::array<log_orientation, 4> new_orientations);
    constexpr bool active() const { return _active; }
    
    placing_log();
};
