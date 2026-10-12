#pragma once

#include "bn_blending_actions.h"
#include "bn_list.h"
#include "bn_sprite_ptr.h"

#include "axe_hit.h"
#include "enums.h"

class board_game_state;
class hand {
private:
    bn::sprite_ptr sprite;
    int current_sprite_index{};
    int column{};
    int row{};

    // I have no idea how to reference `board_largest_axis` here. in a clean way.
    bn::list<axe_hit, 10> previous_axe_hits;
    bn::blending_fade_alpha_loop_action blending_loop{ 45, 1 };

    int swing_animation{};
    bool has_swung{};
    bool _active{};

    move_direction held_direction{};
    int hold_repeat_time{};

    constexpr axe_swing_direction get_swing_direction() const;
    void clear_axe_hits();
    void update_axe_hits(board_game_state &state);
    void move(board_game_state &state);
    void render(board_game_state &state);

public:
    constexpr bool active() const { return _active; }
    void tick(board_game_state &state);
    void enable(board_game_state &state);
    void disable();

    hand(board_game_state &state);
};