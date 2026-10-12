#include "placing_log.h"

#include "bn_sound_items.h"
#include "bn_sprite_items_logs.h"
#include "bn_keypad.h"
#include "bn_log.h"

#include "board_game_state.h"
#include "enums.h"
#include "utils.h"

static void apply_sprite_orientation(bn::sprite_ptr &sprite, log_orientation orientation) {
    switch (orientation) {
    case log_orientation::VERTICAL:
        sprite.set_rotation_angle(0);
        sprite.set_visible(true);
        break;
    case log_orientation::HORIZONTAL:
        sprite.set_rotation_angle(90);
        sprite.set_visible(true);
        break;
    default:
        sprite.set_visible(false);
        break;
    }
}

static constexpr log_orientation swap_orientations(log_orientation orientation) {
    switch (orientation) {
    case log_orientation::HORIZONTAL: return log_orientation::VERTICAL;
    case log_orientation::VERTICAL: return log_orientation::HORIZONTAL;
    default: return orientation;
    }
}

constexpr bn::array<bn::fixed_point, 4> sprite_screen_offsets{ bn::fixed_point{-12, -24}, bn::fixed_point{0, -24}, bn::fixed_point{0, -12}, bn::fixed_point{-12, -12} };
constexpr bn::array<bn::point, 4> sprite_tile_offsets{ bn::point{-1, -1}, bn::point{0, -1}, bn::point{0, 0}, bn::point{-1, 0} };

placing_log::placing_log()
    : sprites({
        // I legit cannot think of a better way of initializing this.
        // Despite how fugly it is.
        bn::sprite_items::logs.create_sprite(sprite_screen_offsets.at(0), 0),
        bn::sprite_items::logs.create_sprite(sprite_screen_offsets.at(1), 0),
        bn::sprite_items::logs.create_sprite(sprite_screen_offsets.at(2), 0),
        bn::sprite_items::logs.create_sprite(sprite_screen_offsets.at(3), 0)
    })
{
    sprites.at(0).set_z_order(-4);
    sprites.at(1).set_z_order(-3);
    sprites.at(2).set_z_order(-1);
    sprites.at(3).set_z_order(-2);
    
    for (auto it = sprites.begin(); it != sprites.end(); it++) {
        it->set_visible(false);
    }
}

void placing_log::tick(board_game_state &state) {
    if (!_active) {
        return;
    }

    // Move left / right
    constexpr int direction_hold_repeat_delay_frames{ 20 };
    constexpr int direction_hold_repeat_spam_frames{ 4 };
    
    move_direction input_direction = move_direction::NONE;
    if (bn::keypad::left_held()) {
        input_direction = move_direction::LEFT;
    } else if (bn::keypad::right_held()) {
        input_direction = move_direction::RIGHT;
    }
    
    bool move_successful = false;
    if (input_direction != move_direction::NONE && (held_direction != input_direction || --hold_repeat_time == 0)) {
        if (input_direction == move_direction::LEFT) {
            if (can_move_into(grid_position + bn::point{ -1, 0 }, state)) {
                grid_position.set_x(grid_position.x() - 1);
                move_successful = true;
            }
        } else {
            if (can_move_into(grid_position + bn::point{ 1, 0 }, state)) {
                grid_position.set_x(grid_position.x() + 1);
                move_successful = true;
            }
        }

        hold_repeat_time = (held_direction != input_direction) ? direction_hold_repeat_delay_frames : direction_hold_repeat_spam_frames;
    }
    held_direction = input_direction;

    // Rotation
    bool tried_to_rotate = false;
    bool rotate_successful = false;
    if (bn::keypad::a_pressed()) {
        tried_to_rotate = true;
        if (can_rotate_cw(state)) {
            rotate_cw();
            rotate_successful = true;
        }
    }
    if (bn::keypad::b_pressed()) {
        tried_to_rotate = true;
        if (can_rotate_ccw(state)) {
            rotate_ccw();
            rotate_successful = true;
        }
    }

    // Falling
    fall_progress += fall_speed;
    if (bn::keypad::down_held()) {
        fall_progress += fall_speed * 7;    
    }

    if (fall_progress >= state.tile_size_pixels().y()) {
        if (can_move_into(grid_position + bn::point{ 0, 1 }, state)) {
            grid_position.set_y(grid_position.y() + 1);
            fall_progress -= state.tile_size_pixels().y();
            landing_delay = 0;
        } else {
            if (++landing_delay > 10) {
                place_onto_board(state);
                landing_delay = 0;
            } else {
                fall_progress = state.tile_size_pixels().y();
            }
        }
    }

    // Sound
    if (move_successful) {
        bn::sound_items::move.play();
    }
    if (tried_to_rotate) {
        if (rotate_successful) {
            bn::sound_items::rotate.play();
        } else {
            bn::sound_items::error.play();
        }
    }

    // Render sprites
    bn::fixed_point render_position = state.to_screen(grid_position);
    render_position.set_y(render_position.y() + fall_progress);

    for (int i = 0; i < sprites.size(); i++) {
        bn::sprite_ptr &sprite = sprites.at(i);
        sprite.set_position(render_position + sprite_screen_offsets.at(i));
    }
}

bool placing_log::can_rotate_cw(const board_game_state &state) const {
    for (int i = 0; i < orientations.size(); i++) {
        if (orientations.at(i) == log_orientation::NONE) {
            continue;
        }
        
        bn::point tile_offset = sprite_tile_offsets.at((i + 1) % orientations.size());
        bn::point check_position = grid_position + tile_offset;
        if (!(state.is_inside_grid(check_position) && !state.at(check_position).occupied())) {
            return false;
        }
    }

    return true;
}

bool placing_log::can_rotate_ccw(const board_game_state &state) const {
    for (int i = 0; i < orientations.size(); i++) {
        if (orientations.at(i) == log_orientation::NONE) {
            continue;
        }
        
        bn::point tile_offset = sprite_tile_offsets.at((i - 1 + orientations.size()) % orientations.size());
        bn::point check_position = grid_position + tile_offset;
        if (!(state.is_inside_grid(check_position) && !state.at(check_position).occupied())) {
            return false;
        }
    }

    return true;
}

bool placing_log::can_move_into(bn::point new_position, const board_game_state &state) const {
    for (int i = 0; i < orientations.size(); i++) {
        if (orientations.at(i) == log_orientation::NONE) {
            continue;
        }

        bn::point check_position = new_position + sprite_tile_offsets.at(i);
        if (!state.is_inside_grid(check_position) || state.at(check_position).occupied()) {
            return false;
        }
    }

    return true;
}

void placing_log::rotate_cw() {
    log_orientation temp = orientations.at(orientations.size() - 1);
    for (int i = 0; i < orientations.size(); i++) { 
        log_orientation current = orientations.at(i);
        orientations.at(i) = swap_orientations(temp);
        temp = current;
        apply_sprite_orientation(sprites.at(i), orientations.at(i));
    }
}

void placing_log::rotate_ccw() { 
    log_orientation temp = orientations.at(0);
    for (int i = orientations.size() - 1; i >= 0; i--) { 
        log_orientation current = orientations.at(i);
        orientations.at(i) = swap_orientations(temp);
        temp = current;
        apply_sprite_orientation(sprites.at(i), orientations.at(i));
    }
}

void placing_log::place_onto_board(board_game_state &state) {
    for (int i = 0; i < orientations.size(); i++) {
        log_orientation orientation = orientations.at(i);
        if (orientation == log_orientation::NONE) {
            continue;
        }

        state.place_log(grid_position + sprite_tile_offsets.at(i), orientation);
    }

    _active = false;
    for (int i = 0; i < sprites.size(); i++) {
        apply_sprite_orientation(sprites.at(i), log_orientation::NONE);
    }

    bn::sound_items::log_land.play();
}

void placing_log::spawn_at_top(board_game_state &state, bn::array<log_orientation, 4> new_orientations) {
    orientations = new_orientations;
    grid_position = { state.width_tiles() / 2, 1 };
    fall_progress = 0;
    _active = true;

    for (int i = 0; i < sprites.size(); i++) {
        apply_sprite_orientation(sprites.at(i), new_orientations.at(i));
    }
}
