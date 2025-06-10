#pragma once

#include "Pattern.h"

class AudioRipplesPattern final : public Pattern
{
    // Ripple sources
    static constexpr uint8_t MAX_RIPPLES = 6;
    uint8_t rippleX[MAX_RIPPLES];
    uint8_t rippleY[MAX_RIPPLES];
    uint8_t rippleRadius[MAX_RIPPLES];
    uint8_t rippleHue[MAX_RIPPLES];
    uint8_t rippleSpeed[MAX_RIPPLES];
    bool rippleActive[MAX_RIPPLES];

    // Visual parameters
    uint8_t fadeAmount = 240;
    uint8_t colorOffset = 0;
    uint8_t colorSpeed = 2;
    uint8_t audioSensitivity = 8;
    uint8_t rippleThickness = 2;

    // Effects
    bool useKaleidoscope = true;
    uint8_t kaleidoscopeMode = 1;
    bool mirrorMode = false;

    void randomize()
    {
        fadeAmount = random8(220, 250);
        colorSpeed = random8(1, 3);
        audioSensitivity = random8(6, 10);
        rippleThickness = random8(1, 4);
        useKaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        mirrorMode = random8(2);
    }

    void initRipple(uint8_t index)
    {
        rippleX[index] = random8(MATRIX_WIDTH);
        rippleY[index] = random8(MATRIX_HEIGHT);
        rippleRadius[index] = 0;
        rippleHue[index] = random8();
        rippleSpeed[index] = random8(1, 4);
        rippleActive[index] = true;
    }

  public:
    static constexpr auto ID = "Audio Ripples";

    AudioRipplesPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        randomize();
        palette = randomPalette();

        // Initialize all ripples as inactive
        for (uint8_t i = 0; i < MAX_RIPPLES; i++)
        {
            rippleActive[i] = false;
        }
    }

    void render() override
    {
        // Change parameters on beat
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }

            // Start new ripples on beat
            for (uint8_t i = 0; i < MAX_RIPPLES; i++)
            {
                if (!rippleActive[i] && random8(3) == 0)
                {
                    initRipple(i);
                    break;
                }
            }
        }

        // Fade background
        Gfx.dim(fadeAmount);

        // Update color cycling
        colorOffset += colorSpeed;

        // Start ripples based on audio peaks
        for (uint8_t i = 0; i < MAX_RIPPLES; i++)
        {
            if (!rippleActive[i])
            {
                uint8_t audioIndex = i * (BINS / MAX_RIPPLES);
                if (Audio.heights8[audioIndex] > (255 / audioSensitivity))
                {
                    rippleX[i] = map8(audioIndex, 0, MATRIX_WIDTH - 1);
                    rippleY[i] = Audio.heights8[audioIndex] >> 3;
                    rippleRadius[i] = 0;
                    rippleHue[i] = colorOffset + (i * 42);
                    rippleSpeed[i] = 1 + (Audio.energy8 >> 6);
                    rippleActive[i] = true;
                }
            }
        }

        // Draw and update ripples
        for (uint8_t i = 0; i < MAX_RIPPLES; i++)
        {
            if (rippleActive[i])
            {
                // Draw ripple rings
                for (uint8_t thickness = 0; thickness < rippleThickness; thickness++)
                {
                    uint8_t currentRadius = rippleRadius[i] + thickness;
                    if (currentRadius > 0 && currentRadius < 32)
                    {
                        // Draw circle using Bresenham-like algorithm
                        int16_t x = currentRadius;
                        int16_t y = 0;
                        int16_t err = 0;

                        uint8_t brightness = 255 - (currentRadius << 3);
                        CRGB color = ColorFromPalette(palette, rippleHue[i] + (currentRadius << 2), brightness);

                        while (x >= y)
                        {
                            // Draw 8 octants
                            Gfx(rippleX[i] + x, rippleY[i] + y) += color;
                            Gfx(rippleX[i] + y, rippleY[i] + x) += color;
                            Gfx(rippleX[i] - y, rippleY[i] + x) += color;
                            Gfx(rippleX[i] - x, rippleY[i] + y) += color;
                            Gfx(rippleX[i] - x, rippleY[i] - y) += color;
                            Gfx(rippleX[i] - y, rippleY[i] - x) += color;
                            Gfx(rippleX[i] + y, rippleY[i] - x) += color;
                            Gfx(rippleX[i] + x, rippleY[i] - y) += color;

                            if (err <= 0)
                            {
                                y++;
                                err += 2 * y + 1;
                            }
                            if (err > 0)
                            {
                                x--;
                                err -= 2 * x + 1;
                            }
                        }
                    }
                }

                // Update ripple
                rippleRadius[i] += rippleSpeed[i];

                // Deactivate if too large
                if (rippleRadius[i] > 30)
                {
                    rippleActive[i] = false;
                }
            }
        }

        // Add some sparkles on strong audio
        if (Audio.energy8 > 150)
        {
            for (uint8_t i = 0; i < 3; i++)
            {
                uint8_t x = random8(MATRIX_WIDTH);
                uint8_t y = random8(MATRIX_HEIGHT);
                Gfx(x, y) += CHSV(colorOffset + random8(64), 255, random8(128, 255));
            }
        }

        // Apply mirror effect occasionally
        if (mirrorMode && Audio.totalBeats % 16 < 8)
        {
            for (uint8_t x = 0; x < MATRIX_CENTER_X; x++)
            {
                for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
                {
                    Gfx(MATRIX_WIDTH - 1 - x, y) = Gfx(x, y);
                }
            }
        }

        // Apply kaleidoscope effect
        if (useKaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);

            // Extra kaleidoscope on strong beats
            if (Audio.isBeat && Audio.energy8 > 200)
            {
                if (random8(2))
                {
                    Gfx.kaleidoscope2();
                }
            }
        }
    }
};
