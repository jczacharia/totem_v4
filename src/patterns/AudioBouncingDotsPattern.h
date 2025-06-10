#pragma once

#include "Pattern.h"

class AudioBouncingDotsPattern final : public Pattern
{
    static constexpr uint8_t MAX_DOTS = 8;

    uint8_t dotX[MAX_DOTS];
    uint8_t dotY[MAX_DOTS];
    uint8_t dotSpeedX[MAX_DOTS];
    uint8_t dotSpeedY[MAX_DOTS];
    uint8_t dotHue[MAX_DOTS];
    uint8_t colorOffset = 0;

  public:
    static constexpr auto ID = "Bouncing Dots";

    AudioBouncingDotsPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        for (uint8_t i = 0; i < MAX_DOTS; i++)
        {
            dotX[i] = random8(MATRIX_WIDTH);
            dotY[i] = random8(MATRIX_HEIGHT);
            dotSpeedX[i] = random8(1, 4);
            dotSpeedY[i] = random8(1, 4);
            dotHue[i] = random8();
        }
        palette = randomPalette();
    }

    void render() override
    {
        // Fade background
        Gfx.dim(240);

        // Update color cycling
        colorOffset += 2;

        // Move and draw dots
        for (uint8_t i = 0; i < MAX_DOTS; i++)
        {
            // Move dots with audio boost
            uint8_t speed = dotSpeedX[i] + (Audio.energy8 >> 6);
            dotX[i] += speed;
            if (dotX[i] >= MATRIX_WIDTH - 1)
            {
                dotX[i] = MATRIX_WIDTH - 1;
                dotSpeedX[i] = -dotSpeedX[i];
            }
            if (dotX[i] == 0)
            {
                dotSpeedX[i] = -dotSpeedX[i];
            }

            speed = dotSpeedY[i] + (Audio.energy8 >> 6);
            dotY[i] += speed;
            if (dotY[i] >= MATRIX_HEIGHT - 1)
            {
                dotY[i] = MATRIX_HEIGHT - 1;
                dotSpeedY[i] = -dotSpeedY[i];
            }
            if (dotY[i] == 0)
            {
                dotSpeedY[i] = -dotSpeedY[i];
            }

            // Draw dot with audio-reactive brightness
            uint8_t brightness = 150 + (Audio.heights8[i * 8] >> 1);
            uint8_t hue = dotHue[i] + colorOffset;
            Gfx(dotX[i], dotY[i]) += ColorFromPalette(palette, hue, brightness);
        }

        // Simple kaleidoscope on beats
        if (Audio.isBeat)
        {
            Gfx.kaleidoscope1();
        }
    }
};
