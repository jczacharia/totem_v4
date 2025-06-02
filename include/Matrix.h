#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <pixeltypes.h>

#include "SmartArray.h"

// Needed for GFX_Lite
extern uint16_t XY(const uint16_t x, const uint16_t y)
{
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) return 0;
    return y * MATRIX_WIDTH + x + 1;
}

class MatrixNoise final : public SmartArray<uint8_t, MATRIX_WIDTH, MATRIX_HEIGHT>
{
public:
    uint32_t noise_x = 0;
    uint32_t noise_y = 0;
    uint32_t noise_z = 0;
    uint32_t noise_scale_x = 0;
    uint32_t noise_scale_y = 0;
    uint8_t noise_smoothing = 0;

    MatrixNoise()
    {
        randomize();
    }

    void randomize()
    {
        noise_smoothing = 200;
        noise_x = random16();
        noise_y = random16();
        noise_z = random16();
        noise_scale_x = 6000;
        noise_scale_y = 6000;
    }

    void fill()
    {
        for (uint16_t x = 0; x < MATRIX_WIDTH; x++)
        {
            const uint32_t i_offset = noise_scale_x * (x - MATRIX_CENTER_Y);
            for (uint16_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                const uint32_t j_offset = noise_scale_y * (y - MATRIX_CENTER_Y);
                byte data = inoise16(noise_x + i_offset, noise_y + j_offset, noise_z) >> 8;
                const uint8_t old_data = (*this)(x, y);
                const uint8_t new_data = scale8(old_data, noise_smoothing) + scale8(data, 256 - noise_smoothing);
                data = new_data;
                (*this)(x, y) = data;
            }
        }
    }
};

class MatrixLeds final : public SmartArray<CRGB, MATRIX_WIDTH, MATRIX_HEIGHT>
{
public:
    void dim(const uint8_t value)
    {
        all([value](CRGB& c) { c.nscale8(value); });
    }
};
