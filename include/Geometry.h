#pragma once

const float GOLDEN_ANGLE_RADIANS = M_PI * (3.0f - sqrtf(5.0f)); // approx 2.39996 radians or 137.5 degrees
constexpr float GOLDEN_RATIO_INV = 1.0f / 1.61803398875f;

struct Point
{
    float x = 0.0f, y = 0.0f;

    Point()
    {
        set(0, 0);
    }

    Point(const float x, const float y)
    {
        set(x, y);
    }

    void set(const float x, const float y)
    {
        this->x = x;
        this->y = y;
    }

    float dist(const Point& other) const
    {
        const float dx = x - other.x;
        const float dy = y - other.y;
        return sqrtf(dx * dx + dy * dy);
    }

    void rotate(const float cx, const float cy, const float angle)
    {
        const float s = sin(angle);
        const float c = cos(angle);

        // translate point back to origin:
        x -= cx;
        y -= cy;

        // rotate point
        const float x_new = x * c - y * s;
        const float y_new = x * s + y * c;

        // translate point back:
        x = x_new + cx;
        y = y_new + cy;
    }
};
