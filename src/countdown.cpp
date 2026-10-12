#include "countdown.h"

#include "bn_sound_items.h"
#include "bn_sprite_items_countdown.h"

countdown::countdown()
    : sprite(bn::sprite_items::countdown.create_sprite())
{
    disable();
}

void countdown::enable() {
    _active = true;
    timer = 60 * 4;
    sprite.set_visible(true);
    bn::sound_items::countdown_blip.play();
}

void countdown::disable() {
    _active = false;
    timer = 0;
    sprite.set_visible(false);
}

void countdown::tick() {
    timer--;

    if (timer == 0) {
        disable();
        return;
    }

    if (timer % 60 == 0) {
        int graphic = 4 - (timer / 60);
        sprite.set_tiles(bn::sprite_items::countdown.tiles_item(), graphic);
        if (graphic == 3) {
            bn::sound_items::countdown_start.play();
        } else {
            bn::sound_items::countdown_blip.play();
        }
    }

    bn::fixed alpha = bn::fixed{timer % 60} / 60;
    alpha *= alpha;
    if (timer < 60) {
        sprite.set_scale(2 - alpha);
    } else {
        sprite.set_scale(bn::fixed{1.5} + alpha);
    }
}
