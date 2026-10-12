#include "hand.h"

#include "bn_array.h"
#include "bn_blending.h"
#include "bn_keypad.h"
#include "bn_fixed.h"
#include "bn_math.h"
#include "bn_sound_items.h"
#include "bn_sprite_items_axe_hands.h"
#include "bn_sprite_tiles_ptr.h"

#include "board_game_state.h"
#include "utils.h"

static constexpr int hand_sprite_index_top{ 0 };
static constexpr int hand_sprite_index_side{ 1 };
static constexpr bn::array<bn::fixed, 11> top_swing_animation{ -5, -30, -50, -70 -80, -90, -100, -100, -80, -20, 120, 100 };

hand::hand(board_game_state &state)
    : sprite(bn::sprite_items::axe_hands.create_sprite({}, hand_sprite_index_top))
{
    disable();
    update_axe_hits(state);
}

void hand::enable(board_game_state &state) {
    bn::blending::set_white_fade_color();
    blending_loop.reset();
    sprite.set_visible(true);
    sprite.set_rotation_angle(0);
    if (!_active) {
        column = state.width_tiles() / 2;
        row = 0;
        _active = true;
    }
    swing_animation = 0;
    update_axe_hits(state);
    render(state);
}

void hand::disable() {
    sprite.set_visible(false);
    clear_axe_hits();
    _active = false;
}

void hand::tick(board_game_state &state) {
    if (swing_animation > 0) {
        swing_animation--;

        bn::fixed angle = top_swing_animation.at(top_swing_animation.size() - swing_animation - 1);
        if (row > 0 && angle > 0) {
            angle /= 2;
        }
        if (!sprite.horizontal_flip()) {
            angle = 360 - angle;
        }
        sprite.set_rotation_angle_safe(angle);

        if (swing_animation == 0) {
            axe_swing_direction swing_direction;
            if (row == 0) {
                swing_direction = axe_swing_direction::DOWN;
            } else if (column == 0) {
                swing_direction = axe_swing_direction::RIGHT;
            } else {
                swing_direction = axe_swing_direction::LEFT;
            }
            
            clear_axe_hits();
            state.swing_axe({ column, row }, swing_direction);
            enable(state);
        }
    } else if (bn::keypad::a_pressed()) {
        if (previous_axe_hits.size() > 1) {
            swing_animation = top_swing_animation.size();
            bn::sound_items::axe_swing.play();
        } else {
            bn::sound_items::error.play();
        }
    } else if (bn::keypad::b_pressed()) {
        disable();
        return;
    } else {
        move(state);
    }

    blending_loop.update();
    render(state);
}

void hand::move(board_game_state &state) {
    // Move left / right
    constexpr int direction_hold_repeat_delay_frames{ 20 };
    constexpr int direction_hold_repeat_spam_frames{ 4 };
    
    move_direction input_direction = move_direction::NONE;
    if (bn::keypad::left_held()) {
        input_direction = move_direction::LEFT;
    } else if (bn::keypad::right_held()) {
        input_direction = move_direction::RIGHT;
    }

    if (input_direction == move_direction::NONE && row != 0) {
        if (bn::keypad::up_held()) {
            input_direction = (column == 0) ? move_direction::RIGHT : move_direction::LEFT;
        } else if (bn::keypad::down_held()) {
            input_direction = (column == 0) ? move_direction::LEFT : move_direction::RIGHT;
        }
    }

    bool moved = false;
    if (input_direction != move_direction::NONE && (held_direction != input_direction || --hold_repeat_time == 0)) {
        if (row == 0) {
            // Move left / right
            int column_target = (input_direction == move_direction::RIGHT) ? state.width_tiles() - 1 : 0;
            if (column != column_target) {
                column = move_towards(column, column_target, 1);
                moved = true;
            }
        }
        if (!moved) {
            // Move up / down
            int row_target = ((input_direction == move_direction::LEFT) ^ (column > 0)) ? state.height_tiles() - 1 : 0;
            if (row != row_target) {
                row = move_towards(row, row_target, 1);
                moved = true;
            }
        }
        hold_repeat_time = (held_direction != input_direction) ? direction_hold_repeat_delay_frames : direction_hold_repeat_spam_frames;
    }
    held_direction = input_direction;

    if (moved) {
        update_axe_hits(state);
        bn::sound_items::move.play();
    }
}

void hand::clear_axe_hits() {
    for (auto it = previous_axe_hits.begin(); it != previous_axe_hits.end(); it++) {
        if (it->slot->sprite.has_value()) {
            it->slot->sprite->set_blending_enabled(false);
        }
    }
    previous_axe_hits.clear();
}

void hand::update_axe_hits(board_game_state &state) {
    clear_axe_hits();

    auto hits = state.find_axe_hits({ column, row }, get_swing_direction());
    if (hits.size() > 1) {
        for (auto it = hits.begin(); it != hits.end(); it++) {
            it->slot->sprite->set_blending_enabled(true);
        }
        previous_axe_hits = hits;
    }
}

void hand::render(board_game_state &state) {
    // Render
    int new_index = (row == 0) ? hand_sprite_index_top : hand_sprite_index_side;
    if (current_sprite_index != new_index) {
        sprite.set_tiles(bn::sprite_items::axe_hands.tiles_item().create_tiles(new_index));
        current_sprite_index = new_index;
    }
    sprite.set_horizontal_flip(column >= state.width_tiles() / 2);

    bn::fixed_point sprite_pos;
    if (row == 0) {
        sprite_pos = state.to_screen({ column, row });
    } else {
        if (column == 0) {
            sprite_pos = state.to_screen({ column - 2, row });
        } else {
            sprite_pos = state.to_screen({ column + 2, row });
        }
    }
    if (row == 0) {
        sprite_pos.set_x(sprite_pos.x() + (sprite.horizontal_flip() ? 8 : -8));
    }

    // Animate bobbing up/down
    constexpr int bobbing_speed{ 35 };
    int offset_sprite = state.frames_since_start() % (bobbing_speed * 2) > bobbing_speed;

    if (row == 0) {
        sprite_pos.set_y(sprite_pos.y() + offset_sprite);
    } else {
        if (column > 0) {
            offset_sprite *= -1;
        } 
        sprite_pos.set_x(sprite_pos.x() + offset_sprite);
    }

    // Apply position
    sprite.set_position(sprite_pos);
}

constexpr axe_swing_direction hand::get_swing_direction() const {
    if (row == 0) {
        return axe_swing_direction::DOWN;
    } else if (column == 0) {
        return axe_swing_direction::RIGHT;
    } else {
        return axe_swing_direction::LEFT;
    }
}