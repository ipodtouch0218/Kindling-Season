#include "board_game_state.h"

#include "bn_affine_mat_attributes.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_items_logs.h"
#include "bn_regular_bg_items_game_board.h"

#include "globals.h"
#include "utils.h"

board_game_state::board_game_state()
    : background(bn::regular_bg_items::game_board.create_bg()),
    hand_instance(*this)
{
    place_log({2, 5}, log_orientation::VERTICAL);
    place_log({2, 6}, log_orientation::HORIZONTAL);
    place_log({2, 7}, log_orientation::HORIZONTAL);
    place_log({3, 7}, log_orientation::HORIZONTAL);
    place_log({3, 6}, log_orientation::VERTICAL);
    place_log({3, 5}, log_orientation::HORIZONTAL);
}

void board_game_state::tick() {
    _frames_since_start++;

    hand_instance.tick(*this);

    if (falling_log_instance.is_active()) {
        falling_log_instance.tick(*this);
        if (!falling_log_instance.is_active()) {
            drop_timer = calculate_drop_timer();
        }
    } else {
        if (--drop_timer <= 0) {
            int rand_x = globals.random.get_int(width_tiles());
            log_orientation rand_orientation = globals.random.get_bool() ? log_orientation::HORIZONTAL : log_orientation::VERTICAL;
            falling_log_instance.activate(*this, rand_x, rand_orientation);
        }
    }
}

void board_game_state::place_log(const bn::point &grid_position, log_orientation orientation) {
    log_slot &slot = at(grid_position);
    slot.occupied = true;
    slot.orientation = orientation;
    slot.sprite = bn::sprite_items::logs.create_sprite(to_screen(grid_position), 0);
    if (orientation == log_orientation::HORIZONTAL) {
        slot.sprite->set_rotation_angle(90);
        slot.sprite->set_y(slot.sprite->y() - 1);
    }
    slot.sprite->set_z_order(grid_position.x() + grid_position.y());
}

void board_game_state::swing_axe(const bn::point &grid_position, axe_swing_direction direction) {

}

bn::fixed calculate_alpha(int frames_since_start) {
    constexpr int frames_for_max_game_speed{ static_cast<int>(59.7275 * 60 * 10) }; // ten minutes
    if (frames_since_start >= frames_for_max_game_speed) {
        return 1;
    } else {
        return bn::fixed{ frames_since_start } / frames_for_max_game_speed;
    }
}

int board_game_state::calculate_drop_timer() const {
    constexpr bn::fixed drop_timer_start{ 90 };
    constexpr bn::fixed drop_timer_end{ 30 };

    // quadratic
    bn::fixed alpha = calculate_alpha(_frames_since_start);
    return lerp(drop_timer_start, drop_timer_end, alpha * alpha).ceil_integer();
}

bn::fixed board_game_state::calculate_drop_speed() const {
    constexpr bn::fixed drop_speed_start{ 0.5 };
    constexpr bn::fixed drop_speed_end{ 2 };

    // linear
    bn::fixed alpha = calculate_alpha(_frames_since_start);
    return lerp(drop_speed_start, drop_speed_end, alpha);
}
