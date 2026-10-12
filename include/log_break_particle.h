#pragma once

#include "bn_sprite_ptr.h"
#include "bn_fixed_point.h"
#include "bn_fixed.h"

struct log_break_particle {
    bn::sprite_ptr sprite;
    bn::fixed_point velocity;

    bool tick();
    log_break_particle(const bn::fixed_point &position, const bn::fixed_point &velocity, int sprite_index, bn::fixed rotation);
};