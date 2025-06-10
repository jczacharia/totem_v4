#pragma once

#include "Pattern.h"

class AudioSpiralsPattern final : public Pattern
{
    uint8_t rotation = 0;
    uint8_t rotationSpeed = 2;
    uint8_t spiralCount = 3;
    uint8_t pointsPerSpiral = 12;
    uint8_t spiralSpread = 85; // 255/3 for even distribution
    uint8_t spiralRadius = 20;
    uint8_t colorOffset = 0;
    uint8_t colorSpeed = 3;
    uint8_t fadeAmount = 230;
    uint8_t audioScale = 4;
    uint8_t beatColorShift = 64;
    bool drawCore = true;
    bool useKaleidoscope = true;
    uint8_t coreSize = 2;

  public:
    static constexpr auto ID = "Audio Spirals";

    AudioSpiralsPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        rotation = random8();
        rotationSpeed = random8(1, 5);
        spiralCount = random8(2, 5);
        pointsPerSpiral = random8(8, 16);
        spiralSpread = 255 / spiralCount;
        spiralRadius = random8(15, 25);
        colorSpeed = random8(2, 6);
        fadeAmount = random8(220, 245);
        audioScale = random8(3, 7);
        beatColorShift = random8(32, 128);
        drawCore = random8(2);
        useKaleidoscope = random8(4) > 0; // 75% chance
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        coreSize = random8(1, 4);
        palette = randomPalette();
    }

    void render() override
    {
        // Beat-triggered parameter changes
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                spiralCount = random8(2, 5);
                spiralSpread = 255 / spiralCount;
                rotationSpeed = random8(1, 5);
            }
            if (Audio.totalBeats % 8 == 0)
            {
                spiralRadius = random8(15, 25);
                audioScale = random8(3, 7);
            }
        }

        // Fade previous frame
        Gfx.dim(fadeAmount);

        // Update rotation based on audio energy
        rotation += rotationSpeed + (Audio.energy8 >> 6);

        // Update color cycling
        colorOffset += colorSpeed;

        // Draw spiral arms
        for (uint8_t spiral = 0; spiral < spiralCount; spiral++)
        {
            uint8_t spiralAngle = rotation + (spiral * spiralSpread);

            for (uint8_t point = 0; point < pointsPerSpiral; point++)
            {
                // Get audio data for this point
                uint8_t audioIndex = (spiral * pointsPerSpiral + point) % BINS;
                uint8_t audioHeight = Audio.heights8[audioIndex] / audioScale;

                // Calculate spiral position using integer math
                uint8_t radius = spiralRadius + (point << 1) + (audioHeight >> 2);
                uint8_t angle = spiralAngle + (point << 3); // point * 8 for spiral twist

                // Convert to cartesian using sin8/cos8
                int8_t dx = ((radius * (cos8(angle) - 128)) >> 7);
                int8_t dy = ((radius * (sin8(angle) - 128)) >> 7);
                int16_t x = MATRIX_CENTER_X + dx;
                int16_t y = MATRIX_CENTER_Y + dy;

                // Calculate color
                uint8_t hue = colorOffset + (spiral << 5) + (point << 2);
                if (Audio.isBeat)
                {
                    hue += beatColorShift;
                }

                uint8_t brightness = 200 + (audioHeight >> 1);
                if (Audio.energy8 > 150)
                {
                    brightness = qadd8(brightness, Audio.energy8 >> 2);
                }

                // Draw the point
                if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                {
                    Gfx(x, y) += ColorFromPalette(palette, hue, brightness);

                    // Add some spread for thicker lines
                    if (x + 1 < MATRIX_WIDTH)
                        Gfx(x + 1, y) += ColorFromPalette(palette, hue + 8, brightness >> 1);
                    if (y + 1 < MATRIX_HEIGHT)
                        Gfx(x, y + 1) += ColorFromPalette(palette, hue + 16, brightness >> 1);
                }
            }
        }

        // Draw pulsing core
        if (drawCore)
        {
            uint8_t coreRadius = coreSize + (Audio.energy8 >> 6);
            uint8_t coreBrightness = 150 + (Audio.energy8 >> 1);

            for (int8_t dx = -coreRadius; dx <= coreRadius; dx++)
            {
                for (int8_t dy = -coreRadius; dy <= coreRadius; dy++)
                {
                    uint8_t distance = abs(dx) + abs(dy); // Manhattan distance
                    if (distance <= coreRadius)
                    {
                        uint8_t fade = 255 - ((distance << 8) / (coreRadius + 1));
                        int16_t x = MATRIX_CENTER_X + dx;
                        int16_t y = MATRIX_CENTER_Y + dy;
                        if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                        {
                            Gfx(x, y) += ColorFromPalette(palette, colorOffset, scale8(coreBrightness, fade));
                        }
                    }
                }
            }
        }

        // Add audio spectrum at edges on strong beats
        if (Audio.energy8 > 180)
        {
            for (uint8_t i = 0; i < MATRIX_WIDTH; i += 4)
            {
                uint8_t height = Audio.heights8[i] >> 4;
                if (height > 2)
                {
                    // Top edge
                    Gfx(i, 0) += ColorFromPalette(palette, colorOffset + (i << 2), height << 4);
                    // Bottom edge
                    Gfx(i, MATRIX_HEIGHT - 1) += ColorFromPalette(palette, colorOffset + (i << 2), height << 4);
                }
            }
        }

        // Apply kaleidoscope effect
        if (useKaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);

            // Extra effect on strong beats
            if (Audio.isBeat && Audio.energy8 > 200)
            {
                if (random8(2))
                {
                    Gfx.kaleidoscope1();
                }
                else
                {
                    Gfx.kaleidoscope2();
                }
            }
        }
    }
};
