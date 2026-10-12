#pragma once

#include "bn_sprite_ptr.h"

class game_over_menu {
private:
    bn::sprite_ptr background;
    bn::sprite_ptr arrows;
    bool _active{};
    int selection{};
    int delay{};

    void move_selection_arrows();

public:
    constexpr bool active() const { return _active; }
    void enable();

    void tick();
    game_over_menu();
};