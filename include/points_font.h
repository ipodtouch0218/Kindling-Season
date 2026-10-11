#pragma once

#include "bn_sprite_font.h"
#include "bn_utf8_characters_map.h"
#include "bn_sprite_items_points_font.h"

constexpr bn::utf8_character point_font_utf8_characters[] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

constexpr int8_t character_widths[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

constexpr bn::span<const bn::utf8_character> points_font_characters_span(point_font_utf8_characters);
constexpr auto points_font_utf8_characters_map = bn::utf8_characters_map<points_font_characters_span>();
constexpr bn::sprite_font points_font{bn::sprite_items::points_font, points_font_utf8_characters_map.reference(), character_widths};