#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <pixeltypes.h>

#include "SmartArray.h"

class MatrixNoise final : public SmartArray<uint8_t, MATRIX_WIDTH, MATRIX_HEIGHT>
{
  public:
    uint32_t noiseX = 0;
    uint32_t noiseY = 0;
    uint32_t noiseZ = 0;
    uint32_t noiseScaleX = 0;
    uint32_t noiseScaleY = 0;
    uint8_t noiseSmoothing = 0;

    MatrixNoise()
    {
        randomize();
    }

    void randomize()
    {
        noiseSmoothing = 200;
        noiseX = random16();
        noiseY = random16();
        noiseZ = random16();
        noiseScaleX = 6000;
        noiseScaleY = 6000;
    }

    void fill()
    {
        for (uint16_t x = 0; x < MATRIX_WIDTH; x++)
        {
            const uint32_t i_offset = noiseScaleX * (x - MATRIX_CENTER_Y);
            for (uint16_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                const uint32_t j_offset = noiseScaleY * (y - MATRIX_CENTER_Y);
                byte data = inoise16(noiseX + i_offset, noiseY + j_offset, noiseZ) >> 8;
                const uint8_t old_data = (*this)(x, y);
                const uint8_t new_data = scale8(old_data, noiseSmoothing) + scale8(data, 256 - noiseSmoothing);
                data = new_data;
                (*this)(x, y) = data;
            }
        }
    }
};

// Needed for GFX_Lite
// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
extern uint16_t XY(const uint16_t x, const uint16_t y)
{
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT)
    {
        return 0;
    }

    return y * MATRIX_WIDTH + x + 1;
}

class MatrixLeds final : public SmartArray<CRGB, MATRIX_WIDTH, MATRIX_HEIGHT>, public GFX
{
  public:
    bool linearBlend = false;

    MatrixLeds()
        : GFX(MATRIX_WIDTH, MATRIX_HEIGHT)
    {
    }

    void dim(const uint8_t value)
    {
        all([value](CRGB &c) { c.nscale8(value); });
    }

    void drawPixel(const int16_t x, const int16_t y, const uint16_t color) override
    {
        if (linearBlend)
        {
            (*this)(x, y) += color;
        }
        else
        {

            (*this)(x, y) = color;
        }
    }

    void drawPixel(const int16_t x, const int16_t y, const CRGB color) override
    {
        if (linearBlend)
        {
            (*this)(x, y) += color;
        }
        else
        {

            (*this)(x, y) = color;
        }
    }

    template <float TGamma, size_t TSteps, size_t TMaxVal>
    constexpr std::array<size_t, TSteps> genGammaTable()
    {
        const float t_max = static_cast<float>(TMaxVal);
        std::array<size_t, TSteps> table{};
        for (size_t i = 0; i < TSteps; ++i)
        {
            const float normalized = static_cast<float>(i) / (TSteps - 1);
            const float corrected = std::pow(normalized, TGamma);
            table[i] = static_cast<size_t>(std::lround(corrected * t_max));
        }
        return table;
    }

    void kaleidoscope1()
    {
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < MATRIX_CENTER_Y; y++)
            {
                (*this)(MATRIX_WIDTH - 1 - x, y) = (*this)(x, y);
                (*this)(MATRIX_WIDTH - 1 - x, MATRIX_HEIGHT - 1 - y) = (*this)(x, y);
                (*this)(x, MATRIX_HEIGHT - 1 - y) = (*this)(x, y);
            }
        }
    }

    void kaleidoscope1Centre()
    {
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < MATRIX_CENTER_Y; y++)
            {
                (*this)(MATRIX_WIDTH - 1 - x, y) = (*this)(x, y);
                (*this)(MATRIX_WIDTH - 1 - x, MATRIX_HEIGHT - 1 - y) = (*this)(x, y);
                (*this)(x, MATRIX_HEIGHT - 1 - y) = (*this)(x, y);
            }
        }
        for (int x = MATRIX_CENTER_X - 1; x >= 0; x--)
        {
            for (int y = MATRIX_CENTER_Y - 1; y >= 0; y--)
            {
                (*this)(x + (MATRIX_WIDTH / 4), y + (MATRIX_HEIGHT / 4)) += (*this)(x, y);
            }
        }
    }

    void kaleidoscope2()
    {
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < MATRIX_CENTER_Y; y++)
            {
                (*this)(MATRIX_WIDTH - 1 - x, y) = (*this)(y, x);
                (*this)(x, MATRIX_HEIGHT - 1 - y) = (*this)(y, x);
                (*this)(MATRIX_WIDTH - 1 - x, MATRIX_HEIGHT - 1 - y) = (*this)(x, y);
            }
        }
    }

    void kaleidoscope3()
    {
        for (int x = 0; x <= MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y <= x && y < MATRIX_HEIGHT; y++)
            {
                (*this)(x, y) = (*this)(y, x);
            }
        }
    }

    void kaleidoscope4Rework()
    {
        for (int x = 0; x <= MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y <= MATRIX_CENTER_Y - x; y++)
            {
                (*this)(x, y) = (*this)(MATRIX_CENTER_Y - y, MATRIX_CENTER_X - x);
            }
        }
    }

    void kaleidoscope4()
    {
        for (int x = 0; x <= MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y <= MATRIX_CENTER_Y - x; y++)
            {
                (*this)(MATRIX_CENTER_Y - y, MATRIX_CENTER_X - x) = (*this)(x, y);
            }
        }
    }

    void kaleidoscopeA1()
    {
        // copy centre left quarter around

        // 1. copy bottom half to above areas
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y; y < MATRIX_HEIGHT - (MATRIX_HEIGHT / 4); y++)
            {
                (*this)(x, y - MATRIX_CENTER_Y) = (*this)(x, y);
                (*this)(x + MATRIX_CENTER_X, y - MATRIX_CENTER_Y) = (*this)(x, y);
            }
        }
        // b. copy top half to below areas
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y - (MATRIX_HEIGHT / 4); y < MATRIX_CENTER_Y; y++)
            {
                (*this)(x, y + MATRIX_CENTER_Y) = (*this)(x, y);
                (*this)(x + MATRIX_CENTER_X, y + MATRIX_CENTER_Y) = (*this)(x, y);
            }
        }

        // iii. copy whole area to the right
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y - (MATRIX_HEIGHT / 4); y < MATRIX_HEIGHT - (MATRIX_HEIGHT / 4); y++)
            {
                (*this)(x + MATRIX_CENTER_X, y) = (*this)(x, y);
            }
        }
    }

    void kaleidoscopeA2()
    {
        // simlilar but mirrors parts on right

        // 1. copy bottom half to above areas
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y; y < MATRIX_HEIGHT - (MATRIX_HEIGHT / 4); y++)
            {
                (*this)(x, y - MATRIX_CENTER_Y) = (*this)(x, y);
                (*this)(x + MATRIX_CENTER_X, y - MATRIX_CENTER_Y) = (*this)(MATRIX_CENTER_X - x, y);
            }
        }
        // b. copy top half to below areas
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y - (MATRIX_HEIGHT / 4); y < MATRIX_CENTER_Y; y++)
            {
                (*this)(x, y + MATRIX_CENTER_Y) = (*this)(x, y);
                (*this)(x + MATRIX_CENTER_X, y + MATRIX_CENTER_Y) = (*this)(MATRIX_CENTER_X - x, y);
            }
        }

        // iii. copy whole area to the right
        for (int x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y - (MATRIX_HEIGHT / 4); y < MATRIX_HEIGHT - (MATRIX_HEIGHT / 4); y++)
            {
                (*this)(x + MATRIX_CENTER_X, y) = (*this)(MATRIX_CENTER_X - x, y);
            }
        }
    }

    void kaleidoscopeB1()
    {
        // copy the quarters of the centre to corners
        constexpr uint8_t matrixQW = MATRIX_WIDTH / 4;
        constexpr uint8_t matrixQH = MATRIX_HEIGHT / 4;

        // 1. copy top left quadrant to top left corner
        for (int x = matrixQW; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_CENTER_Y; y++)
            {
                (*this)(x - matrixQW, y - matrixQH) = (*this)(x, y);
            }
        }
        // 2. copy top right quadrant to top right corner
        for (int x = MATRIX_CENTER_X; x < MATRIX_WIDTH - matrixQW; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_CENTER_Y; y++)
            {
                (*this)(x + matrixQW, y - matrixQH) = (*this)(x, y);
            }
        }
        // 3. copy bottom left quadrant to bottom left corner
        for (int x = matrixQW; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_CENTER_Y; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                (*this)(x - matrixQW, y + matrixQH) = (*this)(x, y);
            }
        }
        // 4. copy bottom right quadrant to bottom right corner
        for (int x = MATRIX_CENTER_X; x < MATRIX_WIDTH - matrixQW; x++)
        {
            for (int y = MATRIX_CENTER_Y; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                (*this)(x + matrixQW, y + matrixQH) = (*this)(x, y);
            }
        }
    }

    void kaleidoscopeB2()
    {
        // copy the halves of the centre to corners
        constexpr uint8_t matrixQW = MATRIX_WIDTH / 4;
        constexpr uint8_t matrixQH = MATRIX_HEIGHT / 4;

        // 1. copy left half to top left corner
        for (int x = matrixQW; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                (*this)(x - matrixQW, y - matrixQH) = (*this)(x, y);
            }
        }
        // 2. copy right half to top right corner
        for (int x = MATRIX_CENTER_X; x < MATRIX_WIDTH - matrixQW; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                (*this)(x + matrixQW, y - matrixQH) = (*this)(x, y);
            }
        }
        // 3. copy left half to bottom left corner
        for (int x = matrixQW; x < MATRIX_CENTER_X; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                (*this)(x - matrixQW, y + matrixQH) = (*this)(x, y);
            }
        }
        // 4. copy right half to bottom right corner
        for (int x = MATRIX_CENTER_X; x < MATRIX_WIDTH - matrixQW; x++)
        {
            for (int y = MATRIX_HEIGHT / 4; y < MATRIX_HEIGHT - matrixQH; y++)
            {
                (*this)(x + matrixQW, y + matrixQH) = (*this)(x, y);
            }
        }
    }

    static constexpr uint8_t KALEIDOSCOPE_COUNT = 9;

    void randomKaleidoscope(const uint8_t id = 0)
    {
        uint8_t randId = id;
        if (id == 0)
            randId = random8(1, KALEIDOSCOPE_COUNT + 1);

        switch (randId)
        {
            case 1: kaleidoscope1(); break;
            case 2: kaleidoscope2(); break;
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
            default: kaleidoscope1(); break;
        }
    }

    void spiralStream(const int x, const int y, const int r, const byte dimm)
    {
        for (int d = r; d >= 0; d--)
        {
            // from the outside to the inside

            for (int i = x - d; i <= x + d; i++)
            {
                (*this)(i, y - d) += (*this)(i + 1, y - d); // lowest row to the right
                (*this)(i, y - d).nscale8(dimm);
            }

            for (int i = y - d; i <= y + d; i++)
            {
                (*this)(x + d, i) += (*this)(x + d, i + 1); // right colum up
                (*this)(x + d, i).nscale8(dimm);
            }

            for (int i = x + d; i >= x - d; i--)
            {
                (*this)(i, y + d) += (*this)(i - 1, y + d); // upper row to the left
                (*this)(i, y + d).nscale8(dimm);
            }

            for (int i = y + d; i >= y - d; i--)
            {
                (*this)(x - d, i) += (*this)(x - d, i - 1); // left colum down
                (*this)(x - d, i).nscale8(dimm);
            }
        }
    }

    // TONY create a square twister to the right or clockwise
    // x and y for center, r for radius
    //
    void spiralStreamVer2(const int x, const int y, const int r, const byte dimm)
    {
        for (int d = 0; d < r; d++)
        {
            // from the outside to the inside

            for (int i = x + d; i >= x - d; i--)
            {
                (*this)(i, y - d) += (*this)(i + 1, y - d); // lowest row to the right
                (*this)(i, y - d).nscale8(dimm);
            }

            for (int i = y + d; i >= y - d; i--)
            {
                (*this)(x + d, i) += (*this)(x + d, i + 1); // right colum up
                (*this)(x + d, i).nscale8(dimm);
            }

            for (int i = x - d; i <= x + d; i++)
            {
                (*this)(i, y + d) += (*this)(i - 1, y + d); // upper row to the left
                (*this)(i, y + d).nscale8(dimm);
            }

            for (int i = y - d; i <= y + d; i++)
            {
                (*this)(x - d, i) += (*this)(x - d, i - 1); // left colum down
                (*this)(x - d, i).nscale8(dimm);
            }
        }
    }

    // expand everything within a circle
    //
    void expand(const int centerX, const int centerY, const int radius, const byte dimm)
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
                (*this)(a + centerX, b + centerY) = (*this)(nextA + centerX, nextB + centerY);
                (*this)(b + centerX, a + centerY) = (*this)(nextB + centerX, nextA + centerY);
                (*this)(-a + centerX, b + centerY) = (*this)(-nextA + centerX, nextB + centerY);
                (*this)(-b + centerX, a + centerY) = (*this)(-nextB + centerX, nextA + centerY);
                (*this)(-a + centerX, -b + centerY) = (*this)(-nextA + centerX, -nextB + centerY);
                (*this)(-b + centerX, -a + centerY) = (*this)(-nextB + centerX, -nextA + centerY);
                (*this)(a + centerX, -b + centerY) = (*this)(nextA + centerX, -nextB + centerY);
                (*this)(b + centerX, -a + centerY) = (*this)(nextB + centerX, -nextA + centerY);

                // dim them
                //
                (*this)(a + centerX, b + centerY).nscale8(dimm);
                (*this)(b + centerX, a + centerY).nscale8(dimm);
                (*this)(-a + centerX, b + centerY).nscale8(dimm);
                (*this)(-b + centerX, a + centerY).nscale8(dimm);
                (*this)(-a + centerX, -b + centerY).nscale8(dimm);
                (*this)(-b + centerX, -a + centerY).nscale8(dimm);
                (*this)(a + centerX, -b + centerY).nscale8(dimm);
                (*this)(b + centerX, -a + centerY).nscale8(dimm);

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
    void streamRight(
        const byte scale,
        const int fromX = 0,
        const int toX = MATRIX_WIDTH,
        const int fromY = 0,
        const int toY = MATRIX_HEIGHT)
    {
        for (int x = fromX + 1; x < toX; x++)
        {
            for (int y = fromY; y < toY; y++)
            {
                (*this)(x, y) += (*this)(x - 1, y);
                (*this)(x, y).nscale8(scale);
            }
        }

        for (int y = fromY; y < toY; y++)
        {
            (*this)(0, y).nscale8(scale);
        }
    }

    // give it a linear tail to the left
    //
    void streamLeft(const byte scale, const int fromX, const int toX, const int fromY, const int toY)
    {
        for (int x = toX; x < fromX; x++)
        {
            for (int y = fromY; y < toY; y++)
            {
                (*this)(x, y) += (*this)(x + 1, y);
                (*this)(x, y).nscale8(scale);
            }
        }

        for (int y = fromY; y < toY; y++)
        {
            (*this)(0, y).nscale8(scale);
        }
    }

    // give it a linear tail downwards
    //
    void streamDown(const byte scale)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            for (int y = 1; y < MATRIX_HEIGHT; y++)
            {
                (*this)(x, y) += (*this)(x, y - 1);
                (*this)(x, y).nscale8(scale);
            }
        }

        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            (*this)(x, 0).nscale8(scale);
        }
    }

    // give it a linear tail upwards
    //
    void streamUp(const byte scale)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
            {
                (*this)(x, y) += (*this)(x, y + 1);
                (*this)(x, y).nscale8(scale);
            }
        }

        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            (*this)(x, MATRIX_HEIGHT - 1).nscale8(scale);
        }
    }

    // give it a linear tail up and to the left
    //
    void streamUpAndLeft(const byte scale)
    {
        for (int x = 0; x < MATRIX_WIDTH - 1; x++)
        {
            for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
            {
                (*this)(x, y) += (*this)(x + 1, y + 1);
                (*this)(x, y).nscale8(scale);
            }
        }

        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            (*this)(x, MATRIX_HEIGHT - 1).nscale8(scale);
        }

        for (int y = 0; y < MATRIX_HEIGHT; y++)
        {
            (*this)(MATRIX_WIDTH - 1, y).nscale8(scale);
        }
    }

    // give it a linear tail up and to the right
    void streamUpAndRight(const byte scale)
    {
        for (int x = 0; x < MATRIX_WIDTH - 1; x++)
        {
            for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
            {
                (*this)(x + 1, y) += (*this)(x, y + 1);
                (*this)(x, y).nscale8(scale);
            }
        }
        // fade the bottom row
        for (int x = 0; x < MATRIX_WIDTH; x++)
            (*this)(x, MATRIX_HEIGHT - 1).nscale8(scale);

        // fade the right column
        for (int y = 0; y < MATRIX_HEIGHT; y++)
            (*this)(MATRIX_WIDTH - 1, y).nscale8(scale);
    }

    // just move everything one line down
    void moveDown()
    {
        for (int y = MATRIX_HEIGHT - 1; y > 0; y--)
        {
            for (int x = 0; x < MATRIX_WIDTH; x++)
            {
                (*this)(x, y) = (*this)(x, y - 1);
            }
        }
    }

    // non leds2 memory version.
    //
    void moveX(const byte delta)
    {
        for (int y = 0; y < MATRIX_HEIGHT; y++)
        {
            // Shift Left:
            // https://codedost.com/c/arraypointers-in-c/c-program-shift-elements-array-left-direction/
            // Computationally heavier but doesn't need an entire leds2 array

            const CRGB tmp = (*this)(0, y);

            for (int m = 0; m < delta; m++)
            {
                // Do this delta time for each row... computationally expensive potentially.

                for (int x = 0; x < MATRIX_WIDTH; x++)
                {
                    (*this)(x, y) = (*this)(x + 1, y);
                }

                (*this)(MATRIX_WIDTH - 1, y) = tmp;
            }
        } // end row loop
    }

    void moveY(const byte delta)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            const CRGB tmp = (*this)(x, 0);

            for (int m = 0; m < delta; m++)
            {
                // Do this delta time for each row... computationally expensive potentially.

                for (int y = 0; y < MATRIX_HEIGHT; y++)
                {
                    (*this)(x, y) = (*this)(x, y + 1);
                }

                (*this)(x, MATRIX_HEIGHT - 1) = tmp;
            }
        }
    }

    // just move everything one line down
    void verticalMoveFrom(const int start, const int end)
    {
        for (int y = end; y > start; y--)
        {
            for (int x = 0; x < MATRIX_WIDTH; x++)
            {
                (*this)(x, y) = (*this)(x, y - 1);
            }
        }
    }

    // copy the rectangle defined with 2 points x0, y0, x1, y1
    // to the rectangle beginning at x2, x3
    void copy(const byte x0, const byte y0, const byte x1, const byte y1, const byte x2, const byte y2)
    {
        for (int y = y0; y < y1 + 1; y++)
        {
            for (int x = x0; x < x1 + 1; x++)
            {
                (*this)(x + x2 - x0, y + y2 - y0) = (*this)(x, y);
            }
        }
    }

    // rotate + copy triangle (MATRIX_CENTER_X*MATRIX_CENTER_X)
    void rotateTriangle()
    {
        for (int x = 1; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < x; y++)
            {
                (*this)(x, 7 - y) = (*this)(7 - x, y);
            }
        }
    }

    // mirror + copy triangle (MATRIX_CENTER_X*MATRIX_CENTER_X)
    void mirrorTriangle()
    {
        for (int x = 1; x < MATRIX_CENTER_X; x++)
        {
            for (int y = 0; y < x; y++)
            {
                (*this)(7 - y, x) = (*this)(7 - x, y);
            }
        }
    }
};
