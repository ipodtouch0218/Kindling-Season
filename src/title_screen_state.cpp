#include "title_screen_state.h"

#include "bn_keypad.h"
#include "bn_regular_bg_items_title.h"
#include "bn_regular_bg_items_title_start.h"
#include "bn_sound_items.h"

extern void change_game_state(game_state);

title_screen_state::title_screen_state()
    : background(bn::regular_bg_items::title.create_bg()),
    press_start(bn::regular_bg_items::title_start.create_bg())
{
    press_start.set_priority(2);
    background.set_priority(3);
}

void title_screen_state::tick() {
    constexpr int flash_rate{ 30 };
    flash_counter++;

    int effective_flash_rate = delay > 0 ? flash_rate / 4 : flash_rate;
    press_start.set_visible(flash_counter % (effective_flash_rate * 2) < effective_flash_rate);

    if (delay > 0) {
        if (--delay == 0) {
            change_game_state(game_state::GAME_BOARD);
        }
        return;
    }

    if (bn::keypad::start_pressed() || bn::keypad::a_pressed()) {
        delay = 60;
        bn::sound_items::menu_ok.play();
    }
}