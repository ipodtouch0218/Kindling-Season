#pragma once

#include "bn_regular_bg_ptr.h"

#include "globals.h"

class title_screen_state : public game_state_runner {
private:
    bn::regular_bg_ptr background;
    bn::regular_bg_ptr press_start;
    int flash_counter{};
    int delay{};

public:
    void tick() override;
    title_screen_state();
};