#pragma once

#include "bn_array.h"
#include "bn_fixed_point.h"
#include "bn_list.h"
#include "bn_optional.h"
#include "bn_point.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_font.h"
#include "bn_sprite_ptr.h"

#include "enums.h"
#include "globals.h"
#include "hand.h"
#include "placing_log.h"

struct log_slot {
    bn::optional<bn::sprite_ptr> sprite;
    log_orientation orientation{};
    
    constexpr bool occupied() const { return orientation != log_orientation::NONE; }
    
    log_slot() = default;
};

struct column_adjustment {
    int starting_row{-1};
    bn::fixed amount;

    column_adjustment() = default;
};

static constexpr bn::point tile_size{ 12, 12 };
static constexpr bn::point board_tiles{ 10, 9 };
static constexpr bn::point board_position{ 60, 36 };
constexpr int board_largest_axis{ bn::max(board_tiles.x(), board_tiles.y()) };

class board_game_state : public game_state_runner {
private:
    bn::array<log_slot, board_tiles.x() * board_tiles.y()> board_state;
    bn::array<column_adjustment, board_tiles.x()> falling_logs;
    
    bn::regular_bg_ptr background;
    
    int axe_counter{};
    int drop_timer{};
    int _frames_since_start{};
    bool board_is_dirty{};
    int columns_with_falling_logs{};

    hand hand_instance;
    placing_log placing_log_instance;

    constexpr bool logs_falling() const { return columns_with_falling_logs > 0; }
    void tick_falling_logs();
    void check_for_falling_logs(int column);
    void update_points(int points);

public:
    log_slot& at(const bn::point &grid_position) { return board_state.at(to_index(grid_position)); }
    const log_slot& at(const bn::point &grid_position) const { return board_state.at(to_index(grid_position)); }
    void place_log(const bn::point &grid_position, log_orientation orientation);

    void swing_axe(const bn::point &grid_position, axe_swing_direction direction);
    bn::list<log_slot*, board_largest_axis> find_axe_hits(const bn::point &grid_position, axe_swing_direction direction);

    int calculate_drop_timer() const;
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
