#pragma once

#include <cmath>

namespace rt {

    struct vec2
    {
        double x;
        double y;

        vec2(double p_v)
            : x(p_v), y(p_v) {}
        vec2(double p_x, double p_y)
            : x(p_x), y(p_y) {}
        vec2 operator-(const vec2& o) const
        {
            return (vec2(x - o.x, y - o.y));
        }
    };

    static double length(vec2 v)
    {
        return (std::sqrt(v.x * v.x + v.y * v.y));
    }

}