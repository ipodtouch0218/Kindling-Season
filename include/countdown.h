#pragma once

#include "bn_sprite_ptr.h"

class countdown {
private:
    bn::sprite_ptr sprite;
    int timer{};
    bool _active{};

public:
    bool active() const { return _active; }
    void enable();
    void disable();
    void tick();
    countdown();
};