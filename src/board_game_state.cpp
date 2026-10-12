#include "board_game_state.h"

#include "bn_affine_mat_attributes.h"
#include "bn_bg_palette_item.h"
#include "bn_memory.h"
#include "bn_optional.h"
#include "bn_sound_items.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_items_logs.h"
#include "bn_string.h"
#include "bn_regular_bg_builder.h"
#include "bn_regular_bg_items_game_board.h"
#include "bn_regular_bg_tiles_items_axe_fill_bar_tiles.h"

#include "globals.h"
#include "utils.h"

static constexpr int axe_timer_length { 900 };
static constexpr int axe_min_blocks{ 7 };
static constexpr int axe_max_blocks{ 10 }; 
static constexpr int log_vertical_sprite_index{ 0 };
static constexpr int log_vertical_break_top_sprite_index{ 1 };
static constexpr int log_vertical_break_bottom_sprite_index{ 2 };
static constexpr int log_horizontal_sprite_index{ 3 };
static constexpr int log_horizontal_break_left_sprite_index{ 4 };
static constexpr int log_horizontal_break_right_sprite_index{ 5 };

static constexpr bn::point point_font_position{ 80, 20 };

static constexpr bn::array<const bn::color, 16> axe_fill_palette_colors{
    bn::color{31, 0, 31},
    bn::color{27, 19, 17},
    bn::color{22, 15, 13}
};
static constexpr bn::span<const bn::color> axe_fill_palette_span{
    axe_fill_palette_colors
};
static constexpr bn::bg_palette_item axe_fill_palette{
    axe_fill_palette_span, bn::bpp_mode::BPP_4
};

board_game_state::board_game_state()
    : background(bn::regular_bg_items::game_board.create_bg()),
    axe_fill_background_palette(axe_fill_palette.create_palette()),
    axe_fill_background_tiles(bn::regular_bg_tiles_items::axe_fill_bar_tiles.create_tiles()),
    axe_fill_background_map(bn::regular_bg_map_ptr::allocate({32, 32}, axe_fill_background_tiles, axe_fill_background_palette)),
    axe_fill_background(bn::regular_bg_builder{axe_fill_background_map}.set_top_left_position({66, 146}).build()),
    hand_instance(*this)
{
    bn::memory::set_half_words(axe_fill_background_map.tiles_offset() | (axe_fill_background_map.palette_banks_offset() << 0xC), 32*32, axe_fill_background_map.vram().value().begin());
    axe_counter = globals.random.get_unbiased_int(axe_min_blocks, axe_max_blocks);
    update_axe_counter_background();
    update_points();
    
    countdown_instance.enable();
}

void board_game_state::tick() {
    if (game_over_menu_instance.active()) {
        game_over_menu_instance.tick();
        return;
    }

    if (countdown_instance.active()) {
        countdown_instance.tick();
        return;
    }

    _frames_since_start++;

    tick_log_break_particles();

    if (delay > 0) {
        delay--;
        return;
    }

    if (axe_swinging()) {
        tick_axe_swing();
        return;
    }

    if (axe_timer > 0) {
        update_axe_counter_background();
        axe_timer--;
    }
    
    if (logs_falling()) {
        tick_falling_logs();
        board_is_dirty = false;
        return;
    }

    if (hand_instance.active()) {
        if (axe_timer == 0) {
            hand_instance.disable();
        } else {
            hand_instance.tick(*this);
        }
    } else {
        axe_timer = 0;
        if (!placing_log_instance.active()) {
            // place new log

            if (at({width_tiles() / 2, 1}).occupied()
                || (width_tiles() % 2 == 0 && at({width_tiles() / 2 - 1, 1}).occupied())) {
                // Game over
                bn::sound_items::game_over.play();
                game_over_menu_instance.enable();
                return;
            }

            bn::array<log_orientation, 4> new_orientations;
            do {
                for (int i = 0; i < new_orientations.size(); i++) {
                    new_orientations[i] = globals.random.get_bool() ? log_orientation::HORIZONTAL : log_orientation::VERTICAL;
                }
                // 3 max
                new_orientations.at(globals.random.get_int(new_orientations.size())) = log_orientation::NONE;
                if (globals.random.get_bool()) {
                    // 1/2 * 3/4 chance to have only 2
                    new_orientations.at(globals.random.get_int(new_orientations.size())) = log_orientation::NONE;
                }
            } while (new_orientations == placing_log_instance.get_orientations()); // no duplicates

            placing_log_instance.spawn_at_top(*this, new_orientations);
            placing_log_instance.set_drop_speed(calculate_drop_speed());
            update_axe_counter_background();
        }
        placing_log_instance.tick(*this);
        if (!placing_log_instance.active()) {
            // Placed new blocks.
            if (--axe_counter < 0) {
                hand_instance.enable(*this);
                combo_counter = 0;
                axe_timer = axe_timer_length;
                axe_counter = globals.random.get_unbiased_int(axe_min_blocks, axe_max_blocks);
            }
        }
    }

    if (board_is_dirty) {
        for (int i = 0; i < width_tiles(); i++) {
            check_for_falling_logs(i);
        }
    }
}

void board_game_state::update_axe_counter_background() {
    constexpr int axe_counter_start_index = 0;
    constexpr int axe_counter_width_tiles = 96 / 8;
    
    bn::fixed fill_percentage;
    if (axe_timer > 0) {
        fill_percentage = bn::fixed{axe_timer} / axe_timer_length;
    } else {
        fill_percentage = 1 - (bn::fixed{axe_counter} / axe_max_blocks);
    }
    int fill_pixels = (fill_percentage * axe_counter_width_tiles * 8).round_integer();
    auto vram = axe_fill_background_map.vram().value();
    for (int i = 0; i < axe_counter_width_tiles; i++) {
        int pixels_this_tile = bn::clamp(fill_pixels, 0, 8);
        fill_pixels -= 8;
        vram[axe_counter_start_index + i] = (axe_fill_background_map.tiles_offset() + pixels_this_tile) | (axe_fill_background_map.palette_banks_offset() << 0xC);
    }
}

void board_game_state::tick_log_break_particles() {
    for (auto it = log_break_particles.begin(); it != log_break_particles.end(); /*nothing*/) {
        if (it->tick()) {
            // Kill
            it = log_break_particles.erase(it);
        } else {
            it++;
        }
    }
}

void board_game_state::tick_falling_logs() {
    constexpr bn::fixed log_fall_speed{ 1.5 };

    bool any_landed = false;
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

                // No new falling logs
                if (adjustment.starting_row == -1) {
                    any_landed = true;
                }
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

    if (any_landed) {
        bn::sound_items::log_land.play();
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
        }
        slot.sprite->set_tiles(bn::sprite_items::logs.tiles_item(), orientation == log_orientation::VERTICAL ? log_vertical_sprite_index : log_horizontal_sprite_index);
        slot.sprite->set_position(to_screen(grid_position));
    }
    board_is_dirty = true;
}

void board_game_state::tick_axe_swing() {
    constexpr int log_break_animation_frames{ 3 };
    constexpr bn::array<bn::sound_item, 10> break_sounds { 
        bn::sound_items::log_break_0, bn::sound_items::log_break_1, bn::sound_items::log_break_2, bn::sound_items::log_break_3, bn::sound_items::log_break_4,
        bn::sound_items::log_break_5, bn::sound_items::log_break_6, bn::sound_items::log_break_7, bn::sound_items::log_break_8, bn::sound_items::log_break_9
    };

    auto hits = find_axe_hits(axe_swing_current_position.value(), axe_swing_current_direction);
    auto it = hits.begin();
    int counter = axe_swing_animation_frames % log_break_animation_frames;
    if (hits.size() == 0) {
        goto end_anim;
    }
    
    if (counter == 0) {
        switch (axe_swing_current_direction) {
        case axe_swing_direction::DOWN:
            it->slot->sprite->set_tiles(bn::sprite_items::logs.tiles_item(), log_vertical_break_top_sprite_index);
            break;
        case axe_swing_direction::RIGHT:
            it->slot->sprite->set_tiles(bn::sprite_items::logs.tiles_item(), log_horizontal_break_left_sprite_index);
            break;
        case axe_swing_direction::LEFT: 
            it->slot->sprite->set_tiles(bn::sprite_items::logs.tiles_item(), log_horizontal_break_right_sprite_index);
            break;
        default:
            break;
        }
    } else if (counter == log_break_animation_frames - 1) {
        if (log_break_particles.size() <= log_break_particles.max_size() - 2) {
            constexpr int log_break_particle_right_index{ 6 };
            constexpr int log_break_particle_left_index{ 7 };

            if (it->slot->orientation == log_orientation::VERTICAL) {
                log_break_particles.push_back({ 
                    it->slot->sprite->position() + bn::fixed_point{-3, 0}, 
                    { globals.random.get_fixed(-3, -1), globals.random.get_fixed(-3, -1.5) }, 
                    log_break_particle_left_index, 
                    0
                });
                log_break_particles.push_back({ 
                    it->slot->sprite->position() + bn::fixed_point{3, 0}, 
                    { globals.random.get_fixed(1, 3), globals.random.get_fixed(-3, -1.5) }, 
                    log_break_particle_right_index, 
                    0 
                });
            } else {
                log_break_particles.push_back({ 
                    it->slot->sprite->position() + bn::fixed_point{0, -3}, 
                    { globals.random.get_fixed(-2, 2), globals.random.get_fixed(-3, -1.5) }, 
                    log_break_particle_right_index, 
                    90 
                });
                log_break_particles.push_back({
                    it->slot->sprite->position() + bn::fixed_point{0, 3},
                    { globals.random.get_fixed(-2, 2), globals.random.get_fixed(1, 2) },
                    log_break_particle_left_index,
                    90 
                });
            }
        }

        place_log(it->position, log_orientation::NONE);
        break_sounds.at(axe_swing_animation_frames / log_break_animation_frames).play();
        points += 10 * (1 << (chain_counter++)) * combo_counter;
        update_points();

        if (hits.size() == 1) {
            // We hit the last log. Bail.
        end_anim:
            axe_swing_current_position = bn::nullopt;
            axe_swing_animation_frames = 0;
            delay = 15;
            return;
        }
    }

    axe_swing_animation_frames++;
}


void board_game_state::swing_axe(const bn::point &grid_position, axe_swing_direction direction) {
    axe_swing_current_position = grid_position;
    axe_swing_current_direction = direction;
    axe_swing_animation_frames = 0;
    chain_counter = 0;
    combo_counter++;
}

bn::list<axe_hit, board_largest_axis> board_game_state::find_axe_hits(const bn::point &grid_position, axe_swing_direction direction) {
    bn::list<axe_hit, bn::max(board_tiles.x(), board_tiles.y())> results;
    if (direction == axe_swing_direction::DOWN) {
        for (int row = 0; row < height_tiles(); row++) {
            bn::point slot_position = { grid_position.x(), row };
            log_slot &slot = at(slot_position);
            if (!slot.occupied() && results.size() > 0) {
                // hit an air gap
                break;
            } else if (slot.orientation == log_orientation::VERTICAL) {
                results.push_back(axe_hit{ &slot, slot_position });
            } else if (slot.orientation == log_orientation::HORIZONTAL) {
                break;
            }
        }
    } else if (direction == axe_swing_direction::RIGHT) {
        for (int column = 0; column < width_tiles(); column++) {
            bn::point slot_position = { column, grid_position.y() };
            log_slot &slot = at(slot_position);
            if (!slot.occupied() && results.size() > 0) {
                // hit an air gap
                break;
            } else if (slot.orientation == log_orientation::HORIZONTAL) {
                results.push_back(axe_hit{ &slot, slot_position });
            } else if (slot.orientation == log_orientation::VERTICAL) {
                break;
            }
        }
    } else if (direction == axe_swing_direction::LEFT) {
        for (int column = width_tiles() - 1; column >= 0; column--) {
            bn::point slot_position = { column, grid_position.y() };
            log_slot &slot = at(slot_position);
            if (!slot.occupied() && results.size() > 0) {
                // hit an air gap
                break;
            } else if (slot.orientation == log_orientation::HORIZONTAL) {
                results.push_back(axe_hit{ &slot, slot_position });
            } else if (slot.orientation == log_orientation::VERTICAL) {
                break;
            }
        }
    }

    return results;
}

void board_game_state::update_points() {
    constexpr const bn::fixed_point points_text_position{ 193, 63 };
    text_generator.set_z_order(2);
    points_text_sprites.clear();
    auto string = bn::to_string<8>(points);
    while (string.length() < 8) {
        string = "0" + string;
    }
    text_generator.generate_top_left(points_text_position, string, points_text_sprites);
}

static bn::fixed calculate_alpha(int frames_since_start) {
    constexpr int frames_for_max_game_speed{ static_cast<int>(59.7275 * 60 * 20) }; // twenty minutes
    if (frames_since_start >= frames_for_max_game_speed) {
        return 1;
    } else {
        return bn::fixed{ frames_since_start } / frames_for_max_game_speed;
    }
}

bn::fixed board_game_state::calculate_drop_speed() const {
    constexpr bn::fixed drop_speed_start{ 0.333 };
    constexpr bn::fixed drop_speed_end{ 2 };

    // linear
    bn::fixed alpha = calculate_alpha(_frames_since_start);
    return lerp(drop_speed_start, drop_speed_end, alpha);
}
