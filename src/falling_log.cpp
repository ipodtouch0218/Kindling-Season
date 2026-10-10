#include "falling_log.h"

#include "bn_sprite_items_logs.h"
#include "bn_log.h"

#include "board_game_state.h"

falling_log::falling_log()
    : sprite(bn::sprite_items::logs.create_sprite({}, 0))
{
    sprite.set_visible(false);
    sprite.set_z_order(-1);
}

void falling_log::tick(board_game_state &state) {
    constexpr bn::fixed sprite_y_check_offset{ 6.5 };

    bn::fixed new_y = sprite.y() + drop_speed;
    bn::point old_grid_pos = state.to_grid({ sprite.x(), sprite.y() + sprite_y_check_offset });
    bn::point new_grid_pos = state.to_grid({ sprite.x(), new_y + sprite_y_check_offset });
    if (old_grid_pos.y() != new_grid_pos.y()) {
        if (new_grid_pos.y() >= 0) {
            if (new_grid_pos.y() >= state.height_tiles() || state.at(new_grid_pos).occupied) {
                state.place_log(old_grid_pos, orientation);
                deactivate();
                return;
            }
        }
        sprite.set_z_order(new_grid_pos.x() + new_grid_pos.y());
    }
    sprite.set_y(new_y);
}

void falling_log::activate(board_game_state &state, int new_grid_x, log_orientation new_orientation) {
    drop_speed = state.calculate_drop_speed();
    grid_x = new_grid_x;
    orientation = new_orientation;

    sprite.set_position(state.to_screen({ new_grid_x, -2 }));
    sprite.set_rotation_angle(new_orientation == log_orientation::VERTICAL ? 0 : 90);
    sprite.set_visible(true);
}

void falling_log::deactivate() {
    sprite.set_visible(false);
}