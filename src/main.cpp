#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "bn_keypad.h"

#include "board_game_state.h"
#include "globals.h"
#include "title_screen_state.h"

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

void change_game_state(game_state new_state) {
    globals.current_state_runner.reset();
    bn::bg_palettes::set_transparent_color({0,0,0});
    bn::core::update(); // FORCE flush tile / bg / palette changes
    bn::core::update();
    switch (new_state) {
    case game_state::TITLE_SCREEN: globals.current_state_runner = bn::make_unique<title_screen_state>(); break;
    case game_state::GAME_BOARD: globals.current_state_runner = bn::make_unique<board_game_state>(); break;
    default: break;
    }
}

int main() {
    bn::core::init();
    change_game_state(game_state::TITLE_SCREEN);

    while(true) {
        bn::core::update();
        update_rng();

        globals.current_state_runner->tick();
    }
}