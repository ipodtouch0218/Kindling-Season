#include "log_break_particle.h"

#include "bn_sprite_items_logs.h"

log_break_particle::log_break_particle(const bn::fixed_point &position, const bn::fixed_point &_velocity, int sprite_index, bn::fixed rotation)
    : sprite(bn::sprite_items::logs.create_sprite(position, sprite_index))
{
    sprite.set_rotation_angle(rotation);
    velocity = _velocity;
}

bool log_break_particle::tick() {
    constexpr bn::fixed kill_y{ 256 };
    constexpr bn::fixed gravity{ 0.1 };

    velocity.set_y(velocity.y() + gravity);
    sprite.set_position(sprite.position() + velocity);
    return sprite.y() > kill_y;
}