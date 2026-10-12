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
    int landing_delay{};

    bool _active{};

    move_direction held_direction{};
    int hold_repeat_time{};

    void place_onto_board(board_game_state &state);
    bool can_rotate_cw(const board_game_state &state) const;
    bool can_rotate_ccw(const board_game_state &state) const;
    bool can_move_into(bn::point grid_position, const board_game_state &state) const;
    void rotate_cw();
    void rotate_ccw();

public:
    void set_drop_speed(bn::fixed value) { fall_speed = value; }
    void tick(board_game_state &state);
    void spawn_at_top(board_game_state &state, bn::array<log_orientation, 4> new_orientations);
    constexpr const bn::array<log_orientation, 4>& get_orientations() const { return orientations; }
    constexpr bool active() const { return _active; }
    
    placing_log();
};
