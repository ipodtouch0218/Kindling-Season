#pragma once

#include "bn_array.h"
#include "bn_fixed_point.h"
#include "bn_optional.h"
#include "bn_point.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_ptr.h"

#include "globals.h"
#include "falling_log.h"
#include "hand.h"
#include "log_orientation.h"

enum class axe_swing_direction {
    RIGHT,
    DOWN,
    LEFT
};

struct log_slot {
    bn::optional<bn::sprite_ptr> sprite;
    log_orientation orientation{};
    bool occupied{};
    
    log_slot() = default;
};

constexpr bn::point tile_size{ 12, 12 };
constexpr bn::point board_tiles{ 10, 8 };
constexpr bn::point board_position{ 60, 48 };

class board_game_state : public game_state_runner {
private:
    bn::array<log_slot, board_tiles.x() * board_tiles.y()> board_state{{}};
    bn::regular_bg_ptr background;
    
    falling_log falling_log_instance;
    int drop_timer{};
    int _frames_since_start{};

    hand hand_instance;

public:
    log_slot& at(const bn::point &position) { return board_state.at(to_index(position)); }
    void place_log(const bn::point &position, log_orientation orientation);
    void swing_axe(const bn::point &position, axe_swing_direction direction);
    
    int calculate_drop_timer() const;
    bn::fixed calculate_drop_speed() const;
    int frames_since_start() const { return _frames_since_start; }

    static constexpr int x() { return board_position.x(); }
    static constexpr int y() { return board_position.y(); }
    static constexpr int width_tiles() { return board_tiles.x(); }
    static constexpr int height_tiles() { return board_tiles.y(); }
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
