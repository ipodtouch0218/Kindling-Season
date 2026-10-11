#include "bn_core.h"
#include "bn_keypad.h"

#include "board_game_state.h"
#include "globals.h"

global_state globals;

static void update_rng() {
    globals.random.update();
    if (bn::keypad::right_held()) {
        globals.random.update();
        globals.random.update();
        globals.random.update();
    }
    if (bn::keypad::left_held()) {
        globals.random.update();
        globals.random.update();
        globals.random.update();
        globals.random.update();
        globals.random.update();
    }
}

int main() {
    bn::core::init();
    globals.current_state_runner = bn::make_unique<board_game_state>();
    
    while(true) {
        bn::core::update();
        update_rng();
        globals.current_state_runner->tick();
    }
}