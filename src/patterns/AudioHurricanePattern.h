#pragma once

#include "Pattern.h"
#include "Util.h"

class AudioHurricanePattern final : public Pattern
{
    uint8_t hurricaneRotation = 0;
    uint8_t colorOffset = 0;
    uint8_t eyeSize = 6;
    uint8_t numArms = 8;

  public:
    static constexpr auto ID = "Hurricane";

    AudioHurricanePattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        eyeSize = random8(4, 8);
        numArms = random8(3, 9); // Random 4-16 arms

        palette = randomPalette();
        kaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        // Beat effects
        if (Audio.isBeat && Audio.totalBeats % 4 == 0)
        {
            kaleidoscope = random8(2);
            kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
            numArms = random8(3, 9); // Change arm count on beats
        }

        // Update rotation with audio
        hurricaneRotation += 1;
        colorOffset += 2;

        // Audio-reactive eye size
        uint8_t currentEyeSize = eyeSize + (Audio.energy8 >> 4);

        // Draw hurricane - scan every pixel
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                // Calculate distance from center
                int16_t dx = x - MATRIX_CENTER_X;
                int16_t dy = y - MATRIX_CENTER_Y;
                uint8_t distance = sqrt16(dx * dx + dy * dy);

                // Hurricane eye (dark center)
                if (distance <= currentEyeSize)
                {
                    uint8_t eyeBrightness = 20 + (distance * 30 / max<uint8_t>(currentEyeSize, 1));
                    Gfx(x, y) = ColorFromPalette(palette, colorOffset + 128, eyeBrightness);
                    continue;
                }

                // Calculate angle using optimized atan2_8 from Util.h
                int8_t dx8 = max<int16_t>(-127, min<int16_t>(127, dx));
                int8_t dy8 = max<int16_t>(-127, min<int16_t>(127, dy));
                uint8_t angle8 = atan2_8(dy8, dx8);

                // Create spiral arms
                // Audio.energy8 controls how tight the spiral is
                uint8_t spiralTightness = (Audio.energy8 >> 1); // Range 8-40
                uint8_t spiralAngle = angle8 + (distance * spiralTightness >> 4) + hurricaneRotation;

                // Create arm pattern
                uint8_t armPattern = sin8(spiralAngle * numArms);

                // Create gradient from center to edge
                uint8_t distanceGradient = min<uint8_t>(distance * 6, 255);

                // Combine arm pattern with gradient
                if (armPattern > 100) // Arms are visible when sin8 > 100
                {
                    uint8_t brightness = (armPattern >> 1) + (distanceGradient >> 2) + (Audio.energy8 >> 3);
                    brightness = min<uint8_t>(brightness, 255);

                    uint8_t hue = colorOffset + (distance << 2) + (spiralAngle >> 3);
                    Gfx(x, y) = ColorFromPalette(palette, hue, brightness);
                }
            }
        }

        // if (kaleidoscope)
        // {
        //     Gfx.randomKaleidoscope(kaleidoscopeMode);
        // }
    }
};
