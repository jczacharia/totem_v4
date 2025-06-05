#pragma once

const float GOLDEN_ANGLE_RADIANS = M_PI * (3.0f - sqrtf(5.0f)); // approx 2.39996 radians or 137.5 degrees
constexpr float GOLDEN_RATIO_INV = 1.0f / 1.61803398875f;

struct Vertex
{
    float x = 0.0f, y = 0.0f, z = 0.0f;

    Vertex()
    {
        this->set(0, 0, 0);
    }

    Vertex(const float x, const float y, const float z)
    {
        this->set(x, y, z);
    }

    void set(const float x, const float y, const float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

struct EdgePoint
{
    int x = 0, y = 0;
    bool visible = false;

    EdgePoint()
    {
        this->set(0, 0);
        this->visible = false;
    }

    void set(const int a, const int b)
    {
        this->x = a;
        this->y = b;
    }
};

struct SquareFace
{
    int length = 0;
    int sommets[4] = {};
    int ed[4] = {};

    SquareFace()
    {
        set(-1, -1, -1, -1);
    }

    SquareFace(const int a, const int b, const int c, const int d)
    {
        this->length = 4;
        this->sommets[0] = a;
        this->sommets[1] = b;
        this->sommets[2] = c;
        this->sommets[3] = d;
    }

    void set(const int a, const int b, const int c, const int d)
    {
        this->length = 4;
        this->sommets[0] = a;
        this->sommets[1] = b;
        this->sommets[2] = c;
        this->sommets[3] = d;
    }
};

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

    float dist(const Point &other) const
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
