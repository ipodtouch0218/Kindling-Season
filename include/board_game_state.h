#pragma once

#include "bn_array.h"
#include "bn_bg_palette_ptr.h"
#include "bn_fixed_point.h"
#include "bn_list.h"
#include "bn_optional.h"
#include "bn_point.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_tiles_ptr.h"
#include "bn_sprite_font.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_vector.h"

#include "axe_hit.h"
#include "countdown.h"
#include "enums.h"
#include "game_over_menu.h"
#include "globals.h"
#include "hand.h"
#include "log_break_particle.h"
#include "placing_log.h"
#include "points_font.h"

struct log_slot {
    bn::optional<bn::sprite_ptr> sprite;
    log_orientation orientation{};
    
    constexpr bool occupied() const { return orientation != log_orientation::NONE; }
    log_slot() = default;
};

struct column_adjustment {
    int starting_row{ -1 };
    bn::fixed amount;

    column_adjustment() = default;
};

static constexpr bn::point tile_size{ 12, 12 };
static constexpr bn::point board_tiles{ 10, 8 };
static constexpr bn::point board_position{ 60, 42 };
constexpr int board_largest_axis{ bn::max(board_tiles.x(), board_tiles.y()) };

class board_game_state : public game_state_runner {
private:
    bn::array<log_slot, board_tiles.x() * board_tiles.y()> board_state;
    bn::array<column_adjustment, board_tiles.x()> falling_logs;
    
    bn::regular_bg_ptr background;

    bn::bg_palette_ptr axe_fill_background_palette;
    bn::regular_bg_tiles_ptr axe_fill_background_tiles;
    bn::regular_bg_map_ptr axe_fill_background_map;
    bn::regular_bg_ptr axe_fill_background;

    bn::list<log_break_particle, board_largest_axis * 2> log_break_particles;

    int axe_counter{};
    int axe_timer{};
    bn::optional<bn::point> axe_swing_current_position;
    axe_swing_direction axe_swing_current_direction;
    int axe_swing_animation_frames{};
    int combo_counter{};
    int chain_counter{};
    int points{};

    bn::sprite_text_generator text_generator{points_font};
    bn::vector<bn::sprite_ptr, 2> points_text_sprites;

    int delay{};

    int _frames_since_start{};
    bool board_is_dirty{};
    int columns_with_falling_logs{};

    hand hand_instance;
    placing_log placing_log_instance;
    game_over_menu game_over_menu_instance;
    countdown countdown_instance;

    void update_axe_counter_background();

    void tick_log_break_particles();

    constexpr bool axe_swinging() const { return axe_swing_current_position.has_value(); }
    void tick_axe_swing();

    constexpr bool logs_falling() const { return columns_with_falling_logs > 0; }
    void tick_falling_logs();
    void check_for_falling_logs(int column);
    void update_points();

public:
    log_slot& at(const bn::point &grid_position) { return board_state.at(to_index(grid_position)); }
    const log_slot& at(const bn::point &grid_position) const { return board_state.at(to_index(grid_position)); }
    void place_log(const bn::point &grid_position, log_orientation orientation);

    void swing_axe(const bn::point &grid_position, axe_swing_direction direction);
    bn::list<axe_hit, board_largest_axis> find_axe_hits(const bn::point &grid_position, axe_swing_direction direction);

    bn::fixed calculate_drop_speed() const;
    int frames_since_start() const { return _frames_since_start; }

    static constexpr int x() { return board_position.x(); }
    static constexpr int y() { return board_position.y(); }
    static constexpr int width_tiles() { return board_tiles.x(); }
    static constexpr int height_tiles() { return board_tiles.y(); }
    static constexpr bn::point tile_size_pixels() { return tile_size; }
    static constexpr bool is_inside_grid(const bn::point &grid_position) { return grid_position.x() >= 0 && grid_position.x() < width_tiles() && grid_position.y() >= 0 && grid_position.y() < height_tiles(); }
    static constexpr int to_index(const bn::point &grid_position) { return grid_position.x() + grid_position.y() * width_tiles(); }
    static constexpr bn::point to_grid(int index) { return { index % width_tiles(), index / width_tiles() }; }
    static constexpr bn::point to_grid(const bn::fixed_point &screen_position) {
        return {
            ((screen_position.x() + 120 - x() - (tile_size.x() / 2)) / tile_size.x()).round_integer(),
            ((screen_position.y() + 80 - y() - (tile_size.y() / 2)) / tile_size.y()).round_integer()
        };
    }
    static constexpr bn::fixed_point to_screen(int index) { return to_screen(to_grid(index)); }
    static constexpr bn::fixed_point to_screen(const bn::point &grid_position) { 
        return {
            x() + (grid_position.x() * tile_size.x()) + (tile_size.x() / 2) - 120,
            y() + (grid_position.y() * tile_size.y()) + (tile_size.y() / 2) - 80
        };
    }

    void tick() override;
    board_game_state();
};
