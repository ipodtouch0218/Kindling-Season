#include "board_game_state.h"

#include "bn_affine_mat_attributes.h"
#include "bn_optional.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_items_logs.h"
#include "bn_regular_bg_items_game_board.h"

#include "globals.h"
#include "utils.h"

static constexpr int axe_min_blocks{ 7 };
static constexpr int axe_max_blocks{ 12 }; 
static constexpr int log_sprite_index{ 0 };
static constexpr int log_break_top_sprite_index{ 1 };
static constexpr int log_break_bottom_sprite_index{ 2 };

static constexpr bn::point point_font_position{ 80, 20 };

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

    // axe_counter = globals.random.get_unbiased_int(axe_min_blocks, axe_max_blocks);
    axe_counter = 3;
}

void board_game_state::tick() {
    _frames_since_start++;
    
    if (logs_falling()) {
        tick_falling_logs();
        board_is_dirty = false;
        return;
    }

    if (hand_instance.active()) {
        hand_instance.tick(*this);
    } else {
        if (!placing_log_instance.active()) {
            bn::array<log_orientation, 4> new_orientations;
            for (int i = 0; i < new_orientations.size(); i++) {
                new_orientations[i] = globals.random.get_bool() ? log_orientation::HORIZONTAL : log_orientation::VERTICAL;
            }
            new_orientations.at(globals.random.get_int(new_orientations.size())) = log_orientation::NONE;
            placing_log_instance.spawn_at_top(*this, new_orientations);
        }
        placing_log_instance.tick(*this);
        if (!placing_log_instance.active()) {
            // Placed new blocks.
            if (--axe_counter <= 0) {
                hand_instance.enable(*this);
            }
        }
    }

    if (board_is_dirty) {
        for (int i = 0; i < width_tiles(); i++) {
            check_for_falling_logs(i);    
        }
    }
}

void board_game_state::tick_falling_logs() {
    constexpr bn::fixed log_fall_speed{ 0.75 };

    for (int column = 0; column < width_tiles(); column++) {
        column_adjustment &adjustment = falling_logs.at(column);
        int original_starting_row = adjustment.starting_row;
        if (adjustment.starting_row != -1) {
            adjustment.amount += log_fall_speed;
            if (adjustment.amount >= tile_size.y()) {
                // Progressed one row, shift rows downwards
                for (int row = adjustment.starting_row; row >= 0; row--) {
                    place_log({ column, row + 1 }, at({ column, row }).orientation);
                } 
                place_log({ column, 0 }, log_orientation::NONE);
                adjustment.amount = 0;
                adjustment.starting_row = -1;
                columns_with_falling_logs--;
                
                check_for_falling_logs(column);
            }

            for (int row = original_starting_row; row >= 0; row--) {
                bn::point grid_position = { column, row };
                log_slot &slot = at(grid_position);
                if (slot.sprite.has_value()) {
                    slot.sprite->set_y(to_screen(grid_position).y() + adjustment.amount);
                }
            }
        }
    }
}

void board_game_state::check_for_falling_logs(int grid_x) {
    bool found_air = false;
    for (int row = height_tiles() - 1; row >= 0; row--) {
        log_slot &slot = at({ grid_x, row });
        if (slot.occupied()) {
            if (found_air) {
                // start log fall at this row
                column_adjustment &adjustment = falling_logs.at(grid_x);
                adjustment.starting_row = row;
                adjustment.amount = 0;
                columns_with_falling_logs++;
                return;
            }
        } else {
            found_air = true;
        }
    }
}

void board_game_state::place_log(const bn::point &grid_position, log_orientation orientation) {
    log_slot &slot = at(grid_position);
    slot.orientation = orientation;

    if (orientation == log_orientation::NONE) {
        slot.sprite = bn::nullopt;
    } else {
        if (!slot.sprite.has_value()) {
            slot.sprite = bn::sprite_items::logs.create_sprite();
            slot.sprite->set_z_order(grid_position.x() + grid_position.y());
        } else {
            slot.sprite->set_tiles(bn::sprite_items::logs.tiles_item(), log_sprite_index);
        }
        slot.sprite->set_position(to_screen(grid_position));
        if (orientation == log_orientation::HORIZONTAL) {
            slot.sprite->set_rotation_angle(90);
        } else {
            slot.sprite->set_rotation_angle(0);
        }
    }
    board_is_dirty = true;
}

void board_game_state::swing_axe(const bn::point &grid_position, axe_swing_direction direction) {

}

// what a chad of a function
bn::list<log_slot*, board_largest_axis> board_game_state::find_axe_hits(const bn::point &grid_position, axe_swing_direction direction) {

    bn::list<log_slot*, bn::max(board_tiles.x(), board_tiles.y())> results;
    if (direction == axe_swing_direction::DOWN) {
        for (int row = 0; row < height_tiles(); row++) {
            log_slot &slot = at({ grid_position.x(), row });
            if (!slot.occupied() && results.size() > 0) {
                // hit an air gap
                break;
            } else if (slot.orientation == log_orientation::VERTICAL) {
                results.push_back(&slot);
            } else if (slot.orientation == log_orientation::HORIZONTAL) {
                break;
            }
        }
    } else if (direction == axe_swing_direction::RIGHT) {
        for (int column = 0; column < width_tiles(); column++) {
            log_slot &slot = at({ column, grid_position.y() });
            if (!slot.occupied() && results.size() > 0) {
                // hit an air gap
                break;
            } else if (slot.orientation == log_orientation::HORIZONTAL) {
                results.push_back(&slot);
            } else if (slot.orientation == log_orientation::VERTICAL) {
                break;
            }
        }
    } else if (direction == axe_swing_direction::LEFT) {
        for (int column = width_tiles() - 1; column >= 0; column--) {
            log_slot &slot = at({ column, grid_position.y() });
            if (!slot.occupied() && results.size() > 0) {
                // hit an air gap
                break;
            } else if (slot.orientation == log_orientation::HORIZONTAL) {
                results.push_back(&slot);
            } else if (slot.orientation == log_orientation::VERTICAL) {
                break;
            }
        }
    }

    return results;
}

#include "bn_sprite_text_generator.h"
void board_game_state::update_points(int points) {
    // TODO
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
