#pragma once

#include <Microphone.h>

#include "Matrix.h"

class PatternContext final : public GFX
{
public:
    MatrixLeds& leds;
    MatrixNoise& noise;
    const AudioContext& audio;
    CRGBPalette16& currentPalette;
    CRGBPalette16& targetPalette;

    explicit PatternContext(
        MatrixLeds& leds,
        MatrixNoise& noise,
        const AudioContext& audio,
        CRGBPalette16& currentPalette,
        CRGBPalette16& targetPalette)
        : GFX(MATRIX_WIDTH,MATRIX_HEIGHT),
          leds(leds),
          noise(noise),
          audio(audio),
          currentPalette(currentPalette),
          targetPalette(targetPalette)
    {
    }

    void drawPixel(const int16_t x, const int16_t y, const uint16_t color) override
    {
        leds(x, y) = color;
    }

    void drawPixel(const int16_t x, const int16_t y, const CRGB color) override
    {
        leds(x, y) = color;
    }

    void kaleidoscope1() const
    {
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < MATRIX_CENTER_Y; y++)
            {
                leds(MATRIX_WIDTH - 1 - x, y) = leds(x, y);
                leds(MATRIX_WIDTH - 1 - x, MATRIX_HEIGHT - 1 - y) = leds(x, y);
                leds(x, MATRIX_HEIGHT - 1 - y) = leds(x, y);
            }
        }
    }

    void kaleidoscope2() const
    {
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < MATRIX_CENTER_Y; y++)
            {
                leds(MATRIX_WIDTH - 1 - x, y) = leds(y, x);
                leds(x, MATRIX_HEIGHT - 1 - y) = leds(y, x);
                leds(MATRIX_WIDTH - 1 - x, MATRIX_HEIGHT - 1 - y) = leds(x, y);
            }
        }
    }

    void kaleidoscope3() const
    {
        for (int x = 0; x <= MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y <= x && y < MATRIX_HEIGHT; y++)
            {
                leds(x, y) = leds(y, x);
            }
        }
    }

    void kaleidoscope4Rework() const
    {
        for (int x = 0; x <= MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y <= MATRIX_CENTER_Y - x; y++)
            {
                leds(x, y) = leds(MATRIX_CENTER_Y - y, MATRIX_CENTER_X - x);
            }
        }
    }

    void kaleidoscope4() const
    {
        for (int x = 0; x <= MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y <= MATRIX_CENTER_Y - x; y++)
            {
                leds(MATRIX_CENTER_Y - y, MATRIX_CENTER_X - x) = leds(x, y);
            }
        }
    }

    void kaleidoscopeA1() const
    {
        // copy centre left quarter around

        // 1. copy bottom half to above areas
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y; y < MATRIX_HEIGHT - (MATRIX_HEIGHT / 4); y++)
            {
                leds(x, y - MATRIX_CENTER_Y) = leds(x, y);
                leds(x + MATRIX_CENTER_X, y - MATRIX_CENTER_Y) = leds(x, y);
            }
        }
        // b. copy top half to below areas
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y - (MATRIX_HEIGHT / 4); y < MATRIX_CENTER_Y; y++)
            {
                leds(x, y + MATRIX_CENTER_Y) = leds(x, y);
                leds(x + MATRIX_CENTER_X, y + MATRIX_CENTER_Y) = leds(x, y);
            }
        }

        // iii. copy whole area to the right
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y - (MATRIX_HEIGHT / 4); y < MATRIX_HEIGHT - (MATRIX_HEIGHT / 4); y++)
            {
                leds(x + MATRIX_CENTER_X, y) = leds(x, y);
            }
        }
    }

    void kaleidoscopeA2() const
    {
        // simlilar but mirrors parts on right

        // 1. copy bottom half to above areas
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y; y < MATRIX_HEIGHT - (MATRIX_HEIGHT / 4); y++)
            {
                leds(x, y - MATRIX_CENTER_Y) = leds(x, y);
                leds(x + MATRIX_CENTER_X, y - MATRIX_CENTER_Y) = leds(MATRIX_CENTER_X - x, y);
            }
        }
        // b. copy top half to below areas
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y - (MATRIX_HEIGHT / 4); y < MATRIX_CENTER_Y; y++)
            {
                leds(x, y + MATRIX_CENTER_Y) = leds(x, y);
                leds(x + MATRIX_CENTER_X, y + MATRIX_CENTER_Y) = leds(MATRIX_CENTER_X - x, y);
            }
        }

        // iii. copy whole area to the right
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y - (MATRIX_HEIGHT / 4); y < MATRIX_HEIGHT - (MATRIX_HEIGHT / 4); y++)
            {
                leds(x + MATRIX_CENTER_X, y) = leds(MATRIX_CENTER_X - x, y);
            }
        }
    }

    void kaleidoscopeB1() const
    {
        // copy the quarters of the centre to corners
        constexpr uint8_t matrixQW = MATRIX_WIDTH / 4;
        constexpr uint8_t matrixQH = MATRIX_HEIGHT / 4;

        // 1. copy top left quadrant to top left corner
        for (int x = matrixQW; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_CENTER_Y; y++)
            {
                leds(x - matrixQW, y - matrixQH) = leds(x, y);
            }
        }
        // 2. copy top right quadrant to top right corner
        for (int x = MATRIX_CENTER_X; x < MATRIX_WIDTH - matrixQW; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_CENTER_Y; y++)
            {
                leds(x + matrixQW, y - matrixQH) = leds(x, y);
            }
        }
        // 3. copy bottom left quadrant to bottom left corner
        for (int x = matrixQW; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                leds(x - matrixQW, y + matrixQH) = leds(x, y);
            }
        }
        // 4. copy bottom right quadrant to bottom right corner
        for (int x = MATRIX_CENTER_X; x < MATRIX_WIDTH - matrixQW; x++)
        {
            for (int y = MATRIX_CENTER_Y; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                leds(x + matrixQW, y + matrixQH) = leds(x, y);
            }
        }
    }

    void kaleidoscopeB2() const
    {
        // copy the halves of the centre to corners
        constexpr uint8_t matrixQW = MATRIX_WIDTH / 4;
        constexpr uint8_t matrixQH = MATRIX_HEIGHT / 4;

        // 1. copy left half to top left corner
        for (int x = matrixQW; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                leds(x - matrixQW, y - matrixQH) = leds(x, y);
            }
        }
        // 2. copy right half to top right corner
        for (int x = MATRIX_CENTER_X; x < MATRIX_WIDTH - matrixQW; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                leds(x + matrixQW, y - matrixQH) = leds(x, y);
            }
        }
        // 3. copy left half to bottom left corner
        for (int x = matrixQW; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                leds(x - matrixQW, y + matrixQH) = leds(x, y);
            }
        }
        // 4. copy right half to bottom right corner
        for (int x = MATRIX_CENTER_X; x < MATRIX_WIDTH - matrixQW; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                leds(x + matrixQW, y + matrixQH) = leds(x, y);
            }
        }
    }

    static constexpr uint8_t KALEIDOSCOPE_COUNT = 9;

    void randomKaleidoscope(const uint8_t id = 0) const
    {
        uint8_t randId = id;
        if (id == 0) randId = random8(1, KALEIDOSCOPE_COUNT + 1);

        switch (randId)
        {
        case 1:
            kaleidoscope1();
            break;
        case 2:
            kaleidoscope2();
            break;
        case 3:
            kaleidoscopeA1();
            kaleidoscope1();
            break;
        case 4:
            kaleidoscopeA2();
            kaleidoscope2();
            break;
        case 5:
            kaleidoscopeB1();
            kaleidoscope1();
            break;
        case 6:
            kaleidoscopeB2();
            kaleidoscope2();
            break;
        case 7:
            // rework kaleidoscope4 toleds mirror bottom right!
            kaleidoscope4Rework();
            kaleidoscope1();
            break;
        case 8:
            // rework kaleidoscope4 toleds mirror bottom right!
            kaleidoscope4Rework();
            kaleidoscope2();
            break;
        case 9:
            kaleidoscope4();
            kaleidoscope1();
            break;
        default:
            kaleidoscope1();
            break;
        }
    }

    FORCE_INLINE_ATTR CRGBPalette16 randomPalette()
    {
        switch (random8(10))
        {
        case 0:
            return RainbowColors_p;

        case 1:
            return OceanColors_p;

        case 2:
            return CloudColors_p;

        case 3:
            return ForestColors_p;

        case 4:
            return PartyColors_p;

        case 5:
            return CRGBPalette16{CRGB::Purple, CRGB::Blue, CRGB::Purple, CRGB::Blue};

        case 6:
            return HeatColors_p;

        case 7:
            return LavaColors_p;

        case 8:
            return CRGBPalette16{CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White};

        default:
            return RainbowColors_p;
        }
    }

    void spiralStream(const int x, const int y, const int r, const byte dimm) const
    {
        for (int d = r; d >= 0; d--)
        {
            // from the outside to the inside

            for (int i = x - d; i <= x + d; i++)
            {
                leds(i, y - d) += leds(i + 1, y - d); // lowest row to the right
                leds(i, y - d).nscale8(dimm);
            }

            for (int i = y - d; i <= y + d; i++)
            {
                leds(x + d, i) += leds(x + d, i + 1); // right colum up
                leds(x + d, i).nscale8(dimm);
            }

            for (int i = x + d; i >= x - d; i--)
            {
                leds(i, y + d) += leds(i - 1, y + d); // upper row to the left
                leds(i, y + d).nscale8(dimm);
            }

            for (int i = y + d; i >= y - d; i--)
            {
                leds(x - d, i) += leds(x - d, i - 1); // left colum down
                leds(x - d, i).nscale8(dimm);
            }
        }
    }


    // TONY create a square twister to the right or clockwise
    // x and y for center, r for radius
    //
    void spiralStreamVer2(const int x, const int y, const int r, const byte dimm) const
    {
        for (int d = 0; d < r; d++)
        {
            // from the outside to the inside

            for (int i = x + d; i >= x - d; i--)
            {
                leds(i, y - d) += leds(i + 1, y - d); // lowest row to the right
                leds(i, y - d).nscale8(dimm);
            }

            for (int i = y + d; i >= y - d; i--)
            {
                leds(x + d, i) += leds(x + d, i + 1); // right colum up
                leds(x + d, i).nscale8(dimm);
            }

            for (int i = x - d; i <= x + d; i++)
            {
                leds(i, y + d) += leds(i - 1, y + d); // upper row to the left
                leds(i, y + d).nscale8(dimm);
            }

            for (int i = y - d; i <= y + d; i++)
            {
                leds(x - d, i) += leds(x - d, i - 1); // left colum down
                leds(x - d, i).nscale8(dimm);
            }
        }
    }

    // expand everything within a circle
    //
    void expand(const int centerX, const int centerY, const int radius,
                const byte dimm) const
    {
        if (radius == 0)
            return;

        int currentRadius = radius;

        while (currentRadius > 0)
        {
            int a = radius, b = 0;
            int radiusError = 1 - a;

            const int nextRadius = currentRadius - 1;
            int nextA = nextRadius - 1, nextB = 0;
            int nextRadiusError = 1 - nextA;

            while (a >= b)
            {
                // move them out one pixel on the radius
                //
                leds(a + centerX, b + centerY) = leds(nextA + centerX, nextB + centerY);
                leds(b + centerX, a + centerY) = leds(nextB + centerX, nextA + centerY);
                leds(-a + centerX, b + centerY) = leds(-nextA + centerX, nextB + centerY);
                leds(-b + centerX, a + centerY) = leds(-nextB + centerX, nextA + centerY);
                leds(-a + centerX, -b + centerY) = leds(-nextA + centerX, -nextB + centerY);
                leds(-b + centerX, -a + centerY) = leds(-nextB + centerX, -nextA + centerY);
                leds(a + centerX, -b + centerY) = leds(nextA + centerX, -nextB + centerY);
                leds(b + centerX, -a + centerY) = leds(nextB + centerX, -nextA + centerY);

                // dim them
                //
                leds(a + centerX, b + centerY).nscale8(dimm);
                leds(b + centerX, a + centerY).nscale8(dimm);
                leds(-a + centerX, b + centerY).nscale8(dimm);
                leds(-b + centerX, a + centerY).nscale8(dimm);
                leds(-a + centerX, -b + centerY).nscale8(dimm);
                leds(-b + centerX, -a + centerY).nscale8(dimm);
                leds(a + centerX, -b + centerY).nscale8(dimm);
                leds(b + centerX, -a + centerY).nscale8(dimm);

                b++;

                if (radiusError < 0)
                {
                    radiusError += 2 * b + 1;
                }
                else
                {
                    a--;
                    radiusError += 2 * (b - a + 1);
                }

                nextB++;

                if (nextRadiusError < 0)
                {
                    nextRadiusError += 2 * nextB + 1;
                }
                else
                {
                    nextA--;
                    nextRadiusError += 2 * (nextB - nextA + 1);
                }
            }

            currentRadius--;
        }
    }

    // give it a linear tail to the right
    //
    void streamRight(const byte scale, const int fromX, const int toX,
                     const int fromY, const int toY) const
    {
        for (int x = fromX + 1; x < toX; x++)
        {
            for (int y = fromY; y < toY; y++)
            {
                leds(x, y) += leds(x - 1, y);
                leds(x, y).nscale8(scale);
            }
        }

        for (int y = fromY; y < toY; y++)
        {
            leds(0, y).nscale8(scale);
        }
    }

    // give it a linear tail to the left
    //
    void streamLeft(const byte scale, const int fromX, const int toX,
                    const int fromY, const int toY) const
    {
        for (int x = toX; x < fromX; x++)
        {
            for (int y = fromY; y < toY; y++)
            {
                leds(x, y) += leds(x + 1, y);
                leds(x, y).nscale8(scale);
            }
        }

        for (int y = fromY; y < toY; y++)
        {
            leds(0, y).nscale8(scale);
        }
    }

    // give it a linear tail downwards
    //
    void streamDown(const byte scale) const
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            for (int y = 1; y < MATRIX_HEIGHT; y++)
            {
                leds(x, y) += leds(x, y - 1);
                leds(x, y).nscale8(scale);
            }
        }

        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            leds(x, 0).nscale8(scale);
        }
    }

    // give it a linear tail upwards
    //
    void streamUp(const byte scale) const
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
            {
                leds(x, y) += leds(x, y + 1);
                leds(x, y).nscale8(scale);
            }
        }

        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            leds(x, MATRIX_HEIGHT - 1).nscale8(scale);
        }
    }

    // give it a linear tail up and to the left
    //
    void streamUpAndLeft(const byte scale) const
    {
        for (int x = 0; x < MATRIX_WIDTH - 1; x++)
        {
            for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
            {
                leds(x, y) += leds(x + 1, y + 1);
                leds(x, y).nscale8(scale);
            }
        }

        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            leds(x, MATRIX_HEIGHT - 1).nscale8(scale);
        }

        for (int y = 0; y < MATRIX_HEIGHT; y++)
        {
            leds(MATRIX_WIDTH - 1, y).nscale8(scale);
        }
    }

    // give it a linear tail up and to the right
    void streamUpAndRight(const byte scale) const
    {
        for (int x = 0; x < MATRIX_WIDTH - 1; x++)
        {
            for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
            {
                leds(x + 1, y) += leds(x, y + 1);
                leds(x, y).nscale8(scale);
            }
        }
        // fade the bottom row
        for (int x = 0; x < MATRIX_WIDTH; x++)
            leds(x, MATRIX_HEIGHT - 1).nscale8(scale);

        // fade the right column
        for (int y = 0; y < MATRIX_HEIGHT; y++)
            leds(MATRIX_WIDTH - 1, y).nscale8(scale);
    }

    // just move everything one line down
    void moveDown() const
    {
        for (int y = MATRIX_HEIGHT - 1; y > 0; y--)
        {
            for (int x = 0; x < MATRIX_WIDTH; x++)
            {
                leds(x, y) = leds(x, y - 1);
            }
        }
    }

    // non leds2 memory version.
    //
    void moveX(const byte delta) const
    {
        for (int y = 0; y < MATRIX_HEIGHT; y++)
        {
            // Shift Left: https://codedost.com/c/arraypointers-in-c/c-program-shift-elements-array-left-direction/
            // Computationally heavier but doesn't need an entire leds2 array

            const CRGB tmp = leds(0, y);

            for (int m = 0; m < delta; m++)
            {
                // Do this delta time for each row... computationally expensive potentially.

                for (int x = 0; x < MATRIX_WIDTH; x++)
                {
                    leds(x, y) = leds(x + 1, y);
                }

                leds(MATRIX_WIDTH - 1, y) = tmp;
            }
        } // end row loop
    }

    void moveY(const byte delta) const
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            const CRGB tmp = leds(x, 0);

            for (int m = 0; m < delta; m++)
            {
                // Do this delta time for each row... computationally expensive potentially.

                for (int y = 0; y < MATRIX_HEIGHT; y++)
                {
                    leds(x, y) = leds(x, y + 1);
                }

                leds(x, MATRIX_HEIGHT - 1) = tmp;
            }
        }
    }

    // just move everything one line down
    void verticalMoveFrom(const int start, const int end) const
    {
        for (int y = end; y > start; y--)
        {
            for (int x = 0; x < MATRIX_WIDTH; x++)
            {
                leds(x, y) = leds(x, y - 1);
            }
        }
    }

    // copy the rectangle defined with 2 points x0, y0, x1, y1
    // to the rectangle beginning at x2, x3
    void copy(const byte x0, const byte y0, const byte x1, const byte y1,
              const byte x2, const byte y2) const
    {
        for (int y = y0; y < y1 + 1; y++)
        {
            for (int x = x0; x < x1 + 1; x++)
            {
                leds(x + x2 - x0, y + y2 - y0) = leds(x, y);
            }
        }
    }

    // rotate + copy triangle (MATRIX_CENTER_X*MATRIX_CENTER_X)
    void rotateTriangle() const
    {
        for (int x = 1; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < x; y++)
            {
                leds(x, 7 - y) = leds(7 - x, y);
            }
        }
    }

    // mirror + copy triangle (MATRIX_CENTER_X*MATRIX_CENTER_X)
    void mirrorTriangle() const
    {
        for (int x = 1; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < x; y++)
            {
                leds(7 - y, x) = leds(7 - x, y);
            }
        }
    }
};

class Pattern
{
    std::string id_;

protected:
    explicit Pattern(std::string name): id_(std::move(name))
    {
    }

public:
    virtual ~Pattern() = default;
    virtual void start(PatternContext& ctx) = 0;
    virtual void render(PatternContext& ctx) = 0;
    [[nodiscard]] const std::string& getId() const { return id_; }
};

template <class T>
concept IPattern = std::is_base_of_v<Pattern, T>;
