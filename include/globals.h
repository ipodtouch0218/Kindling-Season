#pragma once

#include "bn_random.h"
#include "bn_unique_ptr.h"

enum class game_state {
    MAIN_MENU,
    BOARD
};

class game_state_runner {
public:
    virtual void tick() = 0;
    game_state_runner() = default;
    virtual ~game_state_runner() = default;
};

struct global_state {
    game_state state;
    bn::unique_ptr<game_state_runner> current_state_runner;
    bn::random random;
};

extern global_state globals;