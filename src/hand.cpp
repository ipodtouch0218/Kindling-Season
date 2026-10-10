#include "hand.h"

#include "bn_sprite_items_hands.h"
#include "bn_keypad.h"

#include "board_game_state.h"
#include "utils.h"


hand::hand(board_game_state &state)
    : sprite(bn::sprite_items::hands.create_sprite({}, 0))
{
    column = state.width_tiles() / 2;
    row = -1;
}

void hand::tick(board_game_state &state) {

    bool axe_mode = bn::keypad::l_held() && can_enter_axe_mode(state);

    if (!axe_mode) {
        // Can't be on the sides in normal mode
        row = -1;
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

    if (input_direction == move_direction::NONE && axe_mode && row != -1) {
        if (bn::keypad::up_held()) {
            input_direction = (column == 0) ? move_direction::RIGHT : move_direction::LEFT;
        } else if (bn::keypad::down_held()) {
            input_direction = (column == 0) ? move_direction::LEFT : move_direction::RIGHT;
        }
    }

    bool moved = false;
    if (input_direction != move_direction::NONE && (held_direction != input_direction || --hold_repeat_time == 0)) {
        if (row == -1) {
            // Move left / right
            int column_target = (input_direction == move_direction::RIGHT) ? state.width_tiles() - 1 : 0;
            if (column != column_target) {
                column = move_towards(column, column_target, 1);
                moved = true;
            }
        }
        if (!moved && axe_mode) {
            // Move up / down
            int row_target = ((input_direction == move_direction::LEFT) ^ (column > 0)) ? state.height_tiles() - 1 : -1;
            if (row != row_target) {
                row = move_towards(row, row_target, 1);
                moved = true;
            }
        }
        hold_repeat_time = (held_direction != input_direction) ? direction_hold_repeat_delay_frames : direction_hold_repeat_spam_frames;
    }
    held_direction = input_direction;

    // Swing / pick up
    if (bn::keypad::a_pressed()) {

    }

    // Render
    bn::fixed_point sprite_pos;
    if (row == -1) {
        sprite_pos = state.to_screen({ column, row });
    } else {
        if (column == 0) {
            sprite_pos = state.to_screen({ column - 1, row });
        } else {
            sprite_pos = state.to_screen({ column + 1, row });
        }
    }

    // Animate bobbing up/down
    constexpr int bobbing_speed{ 35 };
    int offset_sprite = state.frames_since_start() % (bobbing_speed * 2) > bobbing_speed;

    if (row == -1) {
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

bool hand::can_enter_axe_mode(board_game_state &state) const {
    return !holding_log || !state.at({ 0, column }).occupied;
}