#pragma once

#include "Pattern.h"

class AudioSimpleTrianglesPattern final : public Pattern
{
    uint8_t colorOffset = 0;
    uint8_t triangleSize = 10;
    bool useKaleidoscope = true;

public:
    static constexpr auto ID = "Simple Triangles";

    AudioSimpleTrianglesPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        triangleSize = random8(8, 20);
        useKaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        // Fade background
        Gfx.dim(220);
        
        // Update colors
        colorOffset += 3;
        
        // Draw 3 audio-reactive triangles
        for (uint8_t t = 0; t < 3; t++)
        {
            // Get audio data for triangle size
            uint8_t audioData = Audio.heights8[t * 20] >> 3; // Use different frequency ranges
            uint8_t size = triangleSize + audioData;
            
            // Triangle center positions
            uint8_t centerX = MATRIX_CENTER_X + (t - 1) * 15; // Spread triangles horizontally
            uint8_t centerY = MATRIX_CENTER_Y;
            
            // Calculate triangle points
            uint8_t x1 = centerX;
            uint8_t y1 = centerY - (size >> 1);
            uint8_t x2 = centerX - (size >> 1);
            uint8_t y2 = centerY + (size >> 1);
            uint8_t x3 = centerX + (size >> 1);
            uint8_t y3 = centerY + (size >> 1);
            
            // Draw triangle with audio-reactive color
            uint8_t hue = colorOffset + (t << 6); // Different hue for each triangle
            uint8_t brightness = 180 + (Audio.energy8 >> 2);
            CRGB color = ColorFromPalette(palette, hue, brightness);
            
            Gfx.drawLine(x1, y1, x2, y2, color);
            Gfx.drawLine(x2, y2, x3, y3, color);
            Gfx.drawLine(x3, y3, x1, y1, color);
        }
        
        // Simple kaleidoscope effect
        if (useKaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
}; 