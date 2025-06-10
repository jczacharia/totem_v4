#pragma once

#include "Pattern.h"

class AudioRotatingBarsPattern final : public Pattern
{
    uint8_t rotation = 0;
    uint8_t rotationSpeed = 2;
    uint8_t colorOffset = 0;
    uint8_t barCount = 16;
    bool useKaleidoscope = true;

public:
    static constexpr auto ID = "Rotating Bars";

    AudioRotatingBarsPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        rotationSpeed = random8(1, 4);
        barCount = random8(12, 24);
        useKaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        // Fade background
        Gfx.dim(210);
        
        // Update rotation and colors
        rotation += rotationSpeed + (Audio.energy8 >> 7);
        colorOffset += 2;
        
        // Draw rotating spectrum bars
        for (uint8_t i = 0; i < barCount; i++)
        {
            uint8_t audioIndex = (i * BINS) / barCount; // Map bars to audio bins
            uint8_t audioHeight = Audio.heights8[audioIndex] >> 3; // Scale audio data
            
            if (audioHeight < 2) continue; // Skip quiet frequencies
            
            uint8_t angle = rotation + (i * 255 / barCount); // Evenly space bars around circle
            
            // Calculate line endpoints using sin8/cos8
            int16_t x1 = MATRIX_CENTER_X + ((8 * (cos8(angle) - 128)) >> 7); // Inner point
            int16_t y1 = MATRIX_CENTER_Y + ((8 * (sin8(angle) - 128)) >> 7);
            int16_t x2 = MATRIX_CENTER_X + (((8 + audioHeight) * (cos8(angle) - 128)) >> 7); // Outer point
            int16_t y2 = MATRIX_CENTER_Y + (((8 + audioHeight) * (sin8(angle) - 128)) >> 7);
            
            // Draw bar with audio-reactive color
            uint8_t hue = colorOffset + (i << 3); // Different hue for each bar
            uint8_t brightness = 160 + (Audio.energy8 >> 2);
            CRGB color = ColorFromPalette(palette, hue, brightness);
            
            Gfx.drawLine(x1, y1, x2, y2, color);
        }
        
        // Simple kaleidoscope effect
        if (useKaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
}; 