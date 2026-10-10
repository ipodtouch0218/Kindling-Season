#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_log.h"
#include "bn_unique_ptr.h"
#include "bn_sprite_items_logs.h"

#include "board_game_state.h"
#include "globals.h"

global_state globals;

int main() {
    bn::core::init();
    globals.current_state_runner = bn::make_unique<board_game_state>();
    
    while(true) {
        bn::core::update();
        globals.current_state_runner->tick();
    }
}