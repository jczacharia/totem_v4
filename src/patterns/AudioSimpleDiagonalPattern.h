#pragma once

#include "Pattern.h"

class AudioSimpleDiagonalPattern final : public Pattern
{
    uint8_t colorOffset = 0;
    uint8_t lineSpacing = 4;
    bool flipDirection = false;
    bool useKaleidoscope = true;

  public:
    static constexpr auto ID = "Simple Diagonal";

    AudioSimpleDiagonalPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        lineSpacing = random8(2, 6);
        flipDirection = random8(2);
        useKaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat && Audio.totalBeats % 4 == 0)
        {
            kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        }

        // Fade background
        Gfx.dim(200);

        // Update colors
        colorOffset += 3;

        // Draw diagonal spectrum lines
        for (uint8_t i = 0; i < MATRIX_WIDTH; i += lineSpacing)
        {
            uint8_t audioIndex = (i * BINS) / MATRIX_WIDTH;        // Map position to audio bin
            uint8_t audioHeight = Audio.heights8[audioIndex] >> 2; // Scale audio data

            if (audioHeight < 3)
                continue; // Skip quiet frequencies

            // Calculate diagonal line coordinates
            uint8_t startX, startY, endX, endY;

            if (flipDirection)
            {
                // Top-right to bottom-left diagonal
                startX = i;
                startY = 0;
                endX = i - audioHeight;
                endY = audioHeight;
            }
            else
            {
                // Top-left to bottom-right diagonal
                startX = i;
                startY = 0;
                endX = i + audioHeight;
                endY = audioHeight;
            }

            // Draw line with audio-reactive color
            uint8_t hue = colorOffset + (i << 2);
            CRGB color = ColorFromPalette(palette, hue, Audio.energy8Scaled);

            Gfx.drawLine(startX, startY, endX, endY, color);
        }

        Gfx.randomKaleidoscope(kaleidoscopeMode);
    }
};
