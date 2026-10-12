#include "game_over_menu.h"

#include "bn_keypad.h"
#include "bn_sound_items.h"
#include "bn_sprite_items_game_over.h"
#include "bn_sprite_items_game_over_arrows.h"

#include "enums.h"
#include "globals.h"

extern void change_game_state(game_state);

game_over_menu::game_over_menu()
    : background(bn::sprite_items::game_over.create_sprite({0, 15})),
    arrows(bn::sprite_items::game_over_arrows.create_sprite())
{
    background.set_visible(false);
    background.set_z_order(-32766);
    arrows.set_visible(false);
    arrows.set_z_order(-32767);
}

void game_over_menu::enable() {
    selection = 0;
    _active = true;
    background.set_visible(true);
    arrows.set_visible(true);
    move_selection_arrows();
}

void game_over_menu::tick() {
    if (delay > 0) {
        if (--delay == 0) {
            if (selection == 0) {
                change_game_state(game_state::GAME_BOARD);
            } else {
                change_game_state(game_state::TITLE_SCREEN);
            }
        };
        return;
    }

    if (bn::keypad::down_pressed() && selection == 0) {
        selection = 1;
        move_selection_arrows();
        bn::sound_items::move.play();
    } else if (bn::keypad::up_pressed() && selection == 1) {
        selection = 0;
        move_selection_arrows();
        bn::sound_items::move.play();
    }

    if (bn::keypad::a_pressed()) {
        delay = 15;
        bn::sound_items::menu_ok.play();
    }
}

void game_over_menu::move_selection_arrows() {
    arrows.set_y((selection == 0) ? 28 : 41);
}