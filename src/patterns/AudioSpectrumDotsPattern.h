#pragma once

#include "Pattern.h"

class AudioSpectrumDotsPattern final : public Pattern
{
    uint8_t rotation = 0;
    uint8_t rotationSpeed = 2;
    uint8_t colorOffset = 0;
    uint8_t baseRadius = 8;
    bool pulseMode = false;

public:
    static constexpr auto ID = "Spectrum Dots";

    AudioSpectrumDotsPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        rotationSpeed = random8(1, 5);
        baseRadius = random8(6, 12);
        pulseMode = random8(2);
        palette = randomPalette();
    }

    void render() override
    {
        // Fade background
        Gfx.dim(200);
        
        // Update rotation and colors
        rotation += rotationSpeed;
        colorOffset += 1;
        
        // Draw spectrum dots in circle
        for (uint8_t i = 0; i < BINS; i += 2) // Skip every other bin for performance
        {
            uint8_t audioHeight = Audio.heights8[i] >> 3; // Scale down audio data
            if (audioHeight < 2) continue; // Skip quiet frequencies
            
            uint8_t angle = rotation + (i << 2); // i * 4 for spacing
            uint8_t radius = baseRadius + audioHeight;
            
            // Calculate position using sin8/cos8
            int16_t x = MATRIX_CENTER_X + ((radius * (cos8(angle) - 128)) >> 7);
            int16_t y = MATRIX_CENTER_Y + ((radius * (sin8(angle) - 128)) >> 7);
            
            // Draw dot with audio-reactive brightness
            uint8_t brightness = pulseMode ? (128 + (Audio.energy8 >> 1)) : (180 + audioHeight);
            uint8_t hue = colorOffset + (i << 1);
            
            if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
            {
                Gfx(x, y) += ColorFromPalette(palette, hue, brightness);
            }
        }
        
        // Simple beat effect
        if (Audio.isBeat)
        {
            Gfx.kaleidoscope1();
        }
    }
}; 