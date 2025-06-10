#pragma once

#include "Pattern.h"
#include "Util.h"

class AudioGeometricFlowerPattern final : public Pattern
{
    uint8_t petalCount = 8;
    uint8_t rotation = 0;
    uint8_t colorOffset = 0;
    uint8_t petalLength = 16;
    uint8_t centerRadius = 4;
    uint8_t rotationSpeed = 1;
    uint8_t pulsePhase = 0;
    bool clockwise = true;
    uint8_t petalWidth = 3;

  public:
    static constexpr auto ID = "Geometric Flower";

    AudioGeometricFlowerPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        petalCount = random8(5, 12);
        petalLength = random8(12, 20);
        centerRadius = random8(3, 6);
        rotationSpeed = random8(1, 4);
        petalWidth = random8(2, 5);
        clockwise = random8(2);

        palette = randomPalette();
        kaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        // Beat effects
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                petalCount = random8(5, 12);
                clockwise = random8(2);
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
            }
        }

        // Update rotation and animation
        if (clockwise)
            rotation += rotationSpeed;
        else
            rotation -= rotationSpeed;

        colorOffset += 2;
        pulsePhase += 3;

        // Dim for trail effect
        Gfx.dim(220);

        // Draw center circle
        uint8_t centerBrightness = 100 + (Audio.energy8 >> 2);
        for (int8_t dx = -centerRadius; dx <= centerRadius; dx++)
        {
            for (int8_t dy = -centerRadius; dy <= centerRadius; dy++)
            {
                if (dx * dx + dy * dy <= centerRadius * centerRadius)
                {
                    int16_t x = MATRIX_CENTER_X + dx;
                    int16_t y = MATRIX_CENTER_Y + dy;
                    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                    {
                        Gfx(x, y) = ColorFromPalette(palette, colorOffset, centerBrightness);
                    }
                }
            }
        }

        // Draw petals
        for (uint8_t petal = 0; petal < petalCount; petal++)
        {
            uint8_t petalAngle = rotation + (petal * 255 / petalCount);

            // Get audio data for this petal
            uint8_t audioIndex = (petal * BINS / petalCount) % BINS;
            uint8_t audioHeight = Audio.heights8[audioIndex];

            // Calculate petal length with audio response
            uint8_t currentPetalLength = petalLength + (audioHeight >> 3);
            if (currentPetalLength > 30)
                currentPetalLength = 30;

            // Petal pulse effect
            uint8_t pulseOffset = sin8(pulsePhase + (petal * 32));
            uint8_t petalBrightness = 150 + (pulseOffset >> 2) + (audioHeight >> 3);

            // Draw petal as a series of circles getting smaller toward the tip
            for (uint8_t r = centerRadius + 1; r < currentPetalLength; r++)
            {
                // Calculate petal position
                int8_t dx = ((r * (cos8(petalAngle) - 128)) >> 7);
                int8_t dy = ((r * (sin8(petalAngle) - 128)) >> 7);
                int16_t centerX = MATRIX_CENTER_X + dx;
                int16_t centerY = MATRIX_CENTER_Y + dy;

                // Calculate petal width that tapers toward the tip
                uint8_t currentWidth = (petalWidth * (currentPetalLength - r)) / currentPetalLength;
                if (currentWidth == 0)
                    currentWidth = 1;

                // Draw petal segment
                for (int8_t px = -currentWidth; px <= currentWidth; px++)
                {
                    for (int8_t py = -currentWidth; py <= currentWidth; py++)
                    {
                        if (px * px + py * py <= currentWidth * currentWidth)
                        {
                            int16_t x = centerX + px;
                            int16_t y = centerY + py;

                            if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                            {
                                uint8_t hue = colorOffset + (petal * 24) + (r * 2);
                                uint8_t brightness = (petalBrightness * (currentPetalLength - r)) / currentPetalLength;

                                Gfx(x, y) += ColorFromPalette(palette, hue, brightness);
                            }
                        }
                    }
                }
            }

            int8_t tipX = ((currentPetalLength * (cos8(petalAngle) - 128)) >> 7);
            int8_t tipY = ((currentPetalLength * (sin8(petalAngle) - 128)) >> 7);
            int16_t x = MATRIX_CENTER_X + tipX;
            int16_t y = MATRIX_CENTER_Y + tipY;

            if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
            {
                Gfx(x, y) = ColorFromPalette(palette, colorOffset + (petal * 24) + 128);
            }
        }

        // Apply kaleidoscope effect
        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
};
