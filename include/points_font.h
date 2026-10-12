#pragma once

#include "bn_sprite_font.h"
#include "bn_utf8_characters_map.h"
#include "bn_sprite_items_points_font.h"

constexpr bn::array<const bn::utf8_character, 0> point_font_utf8_characters = { };

constexpr int8_t character_widths[] = {
    2,  // 32
    2,  // 33 !
    2,  // 34 "
    2,  // 35 #
    2,  // 36 $
    2,  // 37 %
    2,  // 38 &
    2,  // 39 '
    2,  // 40 (
    2,  // 41 )
    2,  // 42 *
    2,  // 43 +
    2,  // 44 ,
    2,  // 45 -
    2,  // 46 .
    2,  // 47 /
    5,  // 48 0
    5,  // 49 1
    5,  // 50 2
    5,  // 51 3
    5,  // 52 4
    5,  // 53 5
    5,  // 54 6
    5,  // 55 7
    5,  // 56 8
    5,  // 57 9
    2,  // 58 :
    2,  // 59 ;
    2,  // 60 <
    2,  // 61 =
    2,  // 62 >
    2,  // 63 ?
    2,  // 64 @
    2,  // 65 A
    2,  // 66 B
    2,  // 67 C
    2,  // 68 D
    2,  // 69 E
    2,  // 70 F
    2,  // 71 G
    2,  // 72 H
    2,  // 73 I
    2,  // 74 J
    2,  // 75 K
    2,  // 76 L
    2,  // 77 M
    2,  // 78 N
    2,  // 79 O
    2,  // 80 P
    2,  // 81 Q
    2,  // 82 R
    2,  // 83 S
    2,  // 84 T
    2,  // 85 U
    2,  // 86 V
    2,  // 87 W
    2,  // 88 X
    2,  // 89 Y
    2,  // 90 Z
    2,  // 91 [
    2,  // 92
    2,  // 93 ]
    2,  // 94 ^
    2,  // 95 _
    2,  // 96 `
    2,  // 97 a
    2,  // 98 b
    2,  // 99 c
    2,  // 100 d
    2,  // 101 e
    2,  // 102 f
    2,  // 103 g
    2,  // 104 h
    2,  // 105 i
    2,  // 106 j
    2,  // 107 k
    2,  // 108 l
    2,  // 109 m
    2,  // 110 n
    2,  // 111 o
    2,  // 112 p
    2,  // 113 q
    2,  // 114 r
    2,  // 115 s
    2,  // 116 t
    2,  // 117 u
    2,  // 118 v
    2,  // 119 w
    2,  // 120 x
    2,  // 121 y
    2,  // 122 z
    2,  // 123 {
    2,  // 124 |
    2,  // 125 }
    2,  // 126 ~
};

constexpr bn::span<const bn::utf8_character> points_font_characters_span(point_font_utf8_characters);
constexpr auto points_font_utf8_characters_map = bn::utf8_characters_map<points_font_characters_span>();
constexpr bn::sprite_font points_font{bn::sprite_items::points_font, points_font_utf8_characters_map.reference(), character_widths};