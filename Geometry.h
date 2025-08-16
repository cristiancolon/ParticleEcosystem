#pragma once

#include <cmath>

namespace Geometry {

    struct Vec2 {
        float x;
        float y;
        bool operator==(const Vec2& other) const {
            return x == other.x && y == other.y;
        }
        bool operator!=(const Vec2& other) const {
            return !(*this == other);
        }
        Vec2 operator+(const Vec2& other) const {
            return Vec2{x + other.x, y + other.y};
        }
        Vec2 operator-(const Vec2& other) const {
            return Vec2{x - other.x, y - other.y};
        }
        Vec2 operator*(float scalar) const {
            return Vec2{x * scalar, y * scalar};
        }
    };
}
