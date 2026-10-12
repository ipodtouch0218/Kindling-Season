#pragma once

#include "bn_point.h"

class log_slot;
struct axe_hit {
    log_slot *slot;
    bn::point position;
};