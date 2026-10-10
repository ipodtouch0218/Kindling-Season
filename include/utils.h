#pragma once

#include "bn_fixed.h"
#include "bn_math.h"

template<int Precision>
inline bn::fixed_t<Precision> lerp(bn::fixed_t<Precision> a, bn::fixed_t<Precision> b, bn::fixed_t<Precision> alpha) {
    alpha = bn::clamp(alpha, bn::fixed_t<Precision>{ 0 }, bn::fixed_t<Precision>{ 1 });
    return a + (b - a) * alpha;
}

template<typename T>
inline T move_towards(T start, T target, T step) {
    if (step < 0) {
        step = bn::abs(step);
    }

    T diff = target - start;
    if (diff == 0) {
        return target;
    } else if (diff > 0) {
        // Increase
        T moved = start + step;
        if (moved > target) {
            return target;
        } else {
            return moved;
        }
    } else {
        // Decrease
        T moved = start - step;
        if (moved < target) {
            return target;
        } else {
            return moved;
        }
    }
}