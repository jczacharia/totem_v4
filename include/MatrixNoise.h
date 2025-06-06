#pragma once

#include "SmartArray.h"

template <size_t W, size_t H>
class MatrixNoise final : public SmartArray<uint8_t, W, H>
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
        for (uint16_t x = 0; x < W; x++)
        {
            const uint32_t i_offset = noiseScaleX * (x - H / 2);
            for (uint16_t y = 0; y < H; y++)
            {
                const uint32_t j_offset = noiseScaleY * (y - H / 2);
                byte data = inoise16(noiseX + i_offset, noiseY + j_offset, noiseZ) >> 8;
                const uint8_t old_data = (*this)(x, y);
                const uint8_t new_data = scale8(old_data, noiseSmoothing) + scale8(data, 256 - noiseSmoothing);
                data = new_data;
                (*this)(x, y) = data;
            }
        }
    }

    template <typename TT, size_t CW, size_t CH>
    void apply(SmartArray<TT, CW, CH> &other, const CRGBPalette16 &palette, const uint8_t brightness = 255)
    {
        for (size_t i = 0; i < W; i++)
        {
            for (size_t j = 0; j < H; j++)
            {
                const uint8_t color = (*this)(i, j);
                other(i, j) = ColorFromPalette(palette, color, brightness);
            }
        }
    }
};
