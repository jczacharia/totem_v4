#pragma once

#include "Pattern.h"

class AudioSpiralGalaxyPattern final : public Pattern
{
    // Spiral arm parameters
    static constexpr uint8_t NUM_ARMS = 3;
    static constexpr uint8_t POINTS_PER_ARM = 16;

    uint16_t rotation = 0;
    uint8_t rotationSpeed = 1;
    uint8_t armSpread = 120; // degrees between arms
    uint8_t spiralTightness = 8;
    uint8_t baseRadius = 8;

    // Visual parameters
    uint8_t colorOffset = 0;
    uint8_t colorSpeed = 1;
    uint8_t fadeAmount = 220;
    bool drawCore = true;
    bool drawTrails = true;
    bool reverseRotation = false;
    uint8_t lineThickness = 1;

    // Audio response
    uint8_t audioSensitivity = 6;
    uint8_t beatColorShift = 32;
    uint32_t lastBeatTime = 0;
    uint8_t beatBrightness = 255;

    // Effects
    bool useKaleidoscope = true;
    uint8_t kaleidoscopeMode = 1;
    bool mirrorArms = false;

    void randomize()
    {
        // Randomize parameters
        rotation = random16(360);
        rotationSpeed = random8(1, 4);
        armSpread = 360 / NUM_ARMS; // Even distribution
        spiralTightness = random8(5, 12);
        baseRadius = random8(5, 12);

        colorSpeed = random8(1, 3);
        fadeAmount = random8(200, 240);
        drawCore = random8(2);
        drawTrails = random8(2);
        reverseRotation = random8(2);
        lineThickness = random8(1, 3);

        audioSensitivity = random8(4, 8);
        beatColorShift = random8(16, 48);

        useKaleidoscope = random8(3) > 0; // 66% chance
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        mirrorArms = random8(2);
    }

  public:
    static constexpr auto ID = "Audio Spiral Galaxy";

    AudioSpiralGalaxyPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        randomize();
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        // Fade effect
        if (drawTrails)
        {
            Gfx.dim(fadeAmount);
        }
        else
        {
            Gfx.clear();
        }

        // Update rotation based on audio energy
        if (reverseRotation)
        {
            rotation -= rotationSpeed + (Audio.energy8 >> 6);
        }
        else
        {
            rotation += rotationSpeed + (Audio.energy8 >> 6);
        }
        rotation %= 360;

        // Update color cycling
        colorOffset += colorSpeed;

        // Beat detection effects
        if (Audio.isBeat)
        {
            lastBeatTime = millis();
            beatBrightness = 255;

            // Sometimes change parameters on beat
            if (random8(10) == 0)
            {
                reverseRotation = !reverseRotation;
            }
            if (random8(20) == 0)
            {
                spiralTightness = random8(5, 12);
            }
        }

        // Fade beat brightness
        if (beatBrightness > 200)
        {
            beatBrightness -= 2;
        }

        // Draw spiral arms
        for (uint8_t arm = 0; arm < NUM_ARMS; arm++)
        {
            uint16_t armAngle = rotation + (arm * armSpread);

            // Draw points along each spiral arm
            for (uint8_t point = 0; point < POINTS_PER_ARM; point++)
            {
                // Get audio data for this point
                uint8_t audioIndex = (arm * POINTS_PER_ARM + point) % BINS;
                uint8_t audioHeight = Audio.heights8[audioIndex] / audioSensitivity;

                // Calculate spiral position
                uint8_t radius = baseRadius + point * 2 + (audioHeight >> 2);
                uint16_t angle = armAngle + point * spiralTightness;

                // Convert to cartesian coordinates
                int8_t dx = (radius * (cos8(angle >> 1) - 128)) >> 7;
                int8_t dy = (radius * (sin8(angle >> 1) - 128)) >> 7;
                int16_t x = MATRIX_CENTER_X + dx;
                int16_t y = MATRIX_CENTER_Y + dy;

                // Calculate color
                uint8_t hue = colorOffset + (arm * 85) + (point * 8);
                if (Audio.isBeat && millis() - lastBeatTime < 100)
                {
                    hue += beatColorShift;
                }

                uint8_t brightness = beatBrightness;
                if (audioHeight > 10)
                {
                    brightness = scale8(brightness, 200 + audioHeight);
                }

                // Draw the point with optional thickness
                if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                {
                    for (uint8_t t = 0; t < lineThickness; t++)
                    {
                        if (x + t < MATRIX_WIDTH) Gfx(x + t, y) += ColorFromPalette(palette, hue, brightness);
                        if (y + t < MATRIX_HEIGHT) Gfx(x, y + t) += ColorFromPalette(palette, hue + 8, brightness);
                    }
                }

                // Connect points with lines for smoother spirals
                if (point > 0)
                {
                    // Calculate previous point position
                    uint8_t prevRadius = baseRadius + (point - 1) * 2 +
                                         (Audio.heights8[(audioIndex - 1) % BINS] / audioSensitivity >> 2);
                    uint16_t prevAngle = armAngle + (point - 1) * spiralTightness;
                    int8_t prev_dx = (prevRadius * (cos8(prevAngle >> 1) - 128)) >> 7;
                    int8_t prev_dy = (prevRadius * (sin8(prevAngle >> 1) - 128)) >> 7;
                    int16_t prevX = MATRIX_CENTER_X + prev_dx;
                    int16_t prevY = MATRIX_CENTER_Y + prev_dy;

                    // Draw connecting line
                    Gfx.drawLine(prevX, prevY, x, y, ColorFromPalette(palette, hue - 8, brightness >> 1));
                }

                // Mirror arms effect
                if (mirrorArms)
                {
                    uint8_t mirrorX = MATRIX_WIDTH - 1 - x;
                    uint8_t mirrorY = MATRIX_HEIGHT - 1 - y;
                    Gfx(mirrorX, mirrorY) += ColorFromPalette(palette, hue + 128, brightness >> 1);
                }
            }
        }

        // Draw galaxy core
        if (drawCore)
        {
            uint8_t coreSize = 3 + (Audio.energy8 >> 5);
            uint8_t coreBrightness = 128 + (Audio.energy8 >> 1);

            for (int8_t dx = -coreSize; dx <= coreSize; dx++)
            {
                for (int8_t dy = -coreSize; dy <= coreSize; dy++)
                {
                    uint8_t distance = abs(dx) + abs(dy);
                    if (distance <= coreSize)
                    {
                        uint8_t fade = 255 - (distance * 255 / coreSize);
                        Gfx(MATRIX_CENTER_X + dx, MATRIX_CENTER_Y + dy) +=
                            ColorFromPalette(palette, colorOffset, scale8(coreBrightness, fade));
                    }
                }
            }
        }

        // Add spectrum bars at the bottom like AudioSpectrumPattern
        if (Audio.energy8 > 100)
        {
            for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
            {
                uint8_t height = Audio.heights8[x] >> 4;
                if (height > 0)
                {
                    Gfx(x, MATRIX_HEIGHT - 1) += CHSV(colorOffset + (x << 2), 255, height << 3);
                }
            }
        }

        // Apply kaleidoscope effect
        if (useKaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);

            // Extra kaleidoscope on strong beats like RotatingSpectrumPattern
            if (Audio.isBeat && Audio.energy8 > 200)
            {
                if (random8(2))
                {
                    Gfx.kaleidoscope2();
                }
                else
                {
                    Gfx.kaleidoscope1();
                }
            }
        }
    }
};
