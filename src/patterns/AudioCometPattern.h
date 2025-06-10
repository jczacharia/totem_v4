#pragma once

#include "Pattern.h"

class AudioCometPattern final : public Pattern
{
    static constexpr uint8_t NUM_COMETS = 6;
    static constexpr uint8_t TRAIL_POINTS = 16;
    uint8_t cometX[NUM_COMETS];
    uint8_t cometY[NUM_COMETS];
    uint8_t angle[NUM_COMETS];
    uint8_t speed[NUM_COMETS];
    uint8_t trailX[NUM_COMETS][TRAIL_POINTS];
    uint8_t trailY[NUM_COMETS][TRAIL_POINTS];
    uint8_t trailIndex[NUM_COMETS];
    uint8_t colorOffset = 0;

  public:
    static constexpr auto ID = "Audio Comet";

    AudioCometPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        for (uint8_t i = 0; i < NUM_COMETS; i++)
        {
            cometX[i] = random8(MATRIX_WIDTH);
            cometY[i] = random8(MATRIX_HEIGHT);
            angle[i] = random8();
            speed[i] = random8(1, 3); // Slower speeds
            trailIndex[i] = 0;

            // Initialize trail positions
            for (uint8_t j = 0; j < TRAIL_POINTS; j++)
            {
                trailX[i][j] = cometX[i];
                trailY[i][j] = cometY[i];
            }
        }

        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat && Audio.totalBeats % 4 == 0)
        {
            kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        }

        // Update colors slowly
        colorOffset += 2; // Slower color cycling

        // Process each comet
        for (uint8_t i = 0; i < NUM_COMETS; i++)
        {
            // Beat effects - only affect some comets
            if (Audio.isBeat && i % 2 == 0)
            {
                angle[i] += random8(32);  // Smaller direction changes
                speed[i] = random8(1, 3); // Keep slower speeds
            }

            uint8_t audioSpeed = speed[i] + (Audio.energy8 >> 7); // Slower audio boost

            // Use different audio frequencies for each comet
            uint8_t bassIndex = (i * 10) + 2;
            uint8_t trebleIndex = 50 + (i * 2);
            uint8_t bassInfluence = Audio.heights8[bassIndex] >> 6; // Weaker influence
            uint8_t trebleInfluence = Audio.heights8[trebleIndex] >> 6;
            angle[i] += bassInfluence - trebleInfluence;

            // Move comet using sin8/cos8
            int8_t dx = ((audioSpeed * (cos8(angle[i]) - 128)) >> 7); // Slower movement
            int8_t dy = ((audioSpeed * (sin8(angle[i]) - 128)) >> 7);

            cometX[i] += dx;
            cometY[i] += dy;

            // Wrap around edges
            if (cometX[i] >= MATRIX_WIDTH)
                cometX[i] = 0;
            if (cometY[i] >= MATRIX_HEIGHT)
                cometY[i] = 0;

            // Store current position in trail before moving
            trailX[i][trailIndex[i]] = cometX[i];
            trailY[i][trailIndex[i]] = cometY[i];
            trailIndex[i] = (trailIndex[i] + 1) % TRAIL_POINTS;

            // Draw line trail with diminishing thickness
            for (uint8_t t = 1; t < TRAIL_POINTS; t++)
            {
                uint8_t prevIdx = (trailIndex[i] + TRAIL_POINTS - t) % TRAIL_POINTS;
                uint8_t currIdx = (trailIndex[i] + TRAIL_POINTS - t + 1) % TRAIL_POINTS;

                uint8_t trailBrightness = (Audio.energy8Scaled * (TRAIL_POINTS - t)) / TRAIL_POINTS;
                uint8_t trailHue = colorOffset + (i << 5) + (t << 2);

                // Draw line between trail points
                Gfx.drawLine(
                    trailX[i][prevIdx],
                    trailY[i][prevIdx],
                    trailX[i][currIdx],
                    trailY[i][currIdx],
                    ColorFromPalette(palette, trailHue, trailBrightness));
            }

            // Draw comet head with gradient glow
            uint8_t hue = colorOffset + (i << 5) + (Audio.energy8 >> 3);

            // Gradient glow - multiple circles with decreasing brightness
            for (uint8_t glow = 4; glow > 0; glow--)
            {
                uint8_t glowRadius = ((Audio.energy8Scaled >> 4) + glow);
                uint8_t glowBrightness = (Audio.energy8Scaled * glow) / 8;
                Gfx.drawCircle(
                    cometX[i], cometY[i], glowRadius, ColorFromPalette(palette, hue + (glow << 4), glowBrightness));
            }

            // Bright center point
            Gfx(cometX[i], cometY[i]) += ColorFromPalette(palette, hue, Audio.energy8Scaled);
        }

        // Add occasional sparkles on strong audio
        if (Audio.energy8 > 180 && random8(3) == 0)
        {
            uint8_t sparkleComet = random8(NUM_COMETS);
            uint8_t sparkleHue = colorOffset + (sparkleComet << 5);
            int8_t sparkleX = cometX[sparkleComet] + random8(3) - 1;
            int8_t sparkleY = cometY[sparkleComet] + random8(3) - 1;
            if (sparkleX >= 0 && sparkleX < MATRIX_WIDTH && sparkleY >= 0 && sparkleY < MATRIX_HEIGHT)
            {
                Gfx(sparkleX, sparkleY) += ColorFromPalette(palette, sparkleHue + 128, Audio.energy8Scaled);
            }
        }

        Gfx.randomKaleidoscope(kaleidoscopeMode);
        Gfx.kaleidoscope2();
    }
};
