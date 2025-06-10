#pragma once

#include "Pattern.h"

class AudioMandalaLayersPattern final : public Pattern
{
    static constexpr uint8_t MAX_LAYERS = 6;
    static constexpr uint8_t POINTS_PER_LAYER = 12;

    struct MandalaLayer
    {
        uint8_t depth;    // Z-distance from viewer
        uint8_t rotation; // Current rotation angle
        uint8_t speed;    // Rotation speed
        uint8_t radius;   // Base radius
        uint8_t hue;      // Layer color
        uint8_t pattern;  // Pattern type (0-3)
    };

    MandalaLayer layers[MAX_LAYERS];
    uint8_t colorOffset = 0;
    uint8_t layerSpacing = 30;

  public:
    static constexpr auto ID = "Mandala Layers";

    AudioMandalaLayersPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        layerSpacing = random8(15, 25);

        for (uint8_t i = 0; i < MAX_LAYERS; i++)
        {
            layers[i].depth = 60 + (i * layerSpacing);
            layers[i].rotation = random8();
            layers[i].speed = random8(1, 4);
            layers[i].radius = random8(18, 35); // Larger radius to fill screen
            layers[i].hue = random8();
            layers[i].pattern = random8(4);
        }

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
            for (uint8_t i = 0; i < MAX_LAYERS; i++)
            {
                layers[i].pattern = random8(4);
                layers[i].speed = random8(1, 4);
            }
        }

        // Fade background
        Gfx.dim(210);

        // Update colors
        colorOffset += 3;

        // Draw layers from back to front
        for (uint8_t layer = 0; layer < MAX_LAYERS; layer++)
        {
            MandalaLayer &l = layers[layer];

            // Update rotation
            l.rotation += l.speed + (Audio.energy8 >> 7);

            // Calculate perspective scale with wider field of view
            uint8_t scale = (250 * 100) / (l.depth + 40);
            uint8_t effectiveRadius = (l.radius * scale) >> 8;

            // Audio-reactive radius modulation
            uint8_t audioMod = Audio.heights8[layer * 16] >> 4;
            effectiveRadius += audioMod;

            // Draw mandala pattern based on type
            for (uint8_t point = 0; point < POINTS_PER_LAYER; point++)
            {
                uint8_t angle = l.rotation + (point * 255 / POINTS_PER_LAYER);

                // Calculate base position
                int16_t x = MATRIX_CENTER_X + ((effectiveRadius * (cos8(angle) - 128)) >> 7);
                int16_t y = MATRIX_CENTER_Y + ((effectiveRadius * (sin8(angle) - 128)) >> 7);

                // Pattern variations
                switch (l.pattern)
                {
                    case 0: // Simple dots
                        if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                        {
                            uint8_t brightness = 150 + (Audio.energy8 >> 2);
                            Gfx(x, y) += ColorFromPalette(palette, l.hue + colorOffset, brightness);
                        }
                        break;

                    case 1: // Lines to center
                        Gfx.drawLine(
                            MATRIX_CENTER_X,
                            MATRIX_CENTER_Y,
                            x,
                            y,
                            ColorFromPalette(palette, l.hue + colorOffset + (point << 3), 100));
                        break;

                    case 2: // Petals (lines between adjacent points)
                    {
                        uint8_t nextAngle = l.rotation + ((point + 1) * 255 / POINTS_PER_LAYER);
                        int16_t x2 = MATRIX_CENTER_X + ((effectiveRadius * (cos8(nextAngle) - 128)) >> 7);
                        int16_t y2 = MATRIX_CENTER_Y + ((effectiveRadius * (sin8(nextAngle) - 128)) >> 7);
                        Gfx.drawLine(x, y, x2, y2, ColorFromPalette(palette, l.hue + colorOffset + (point << 4), 120));
                    }
                    break;

                    case 3: // Double radius (inner and outer rings)
                    {
                        uint8_t innerRadius = effectiveRadius >> 1;
                        int16_t ix = MATRIX_CENTER_X + ((innerRadius * (cos8(angle) - 128)) >> 7);
                        int16_t iy = MATRIX_CENTER_Y + ((innerRadius * (sin8(angle) - 128)) >> 7);

                        if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                            Gfx(x, y) += ColorFromPalette(palette, l.hue + colorOffset, 120);
                        if (ix >= 0 && ix < MATRIX_WIDTH && iy >= 0 && iy < MATRIX_HEIGHT)
                            Gfx(ix, iy) += ColorFromPalette(palette, l.hue + colorOffset + 128, 80);
                    }
                    break;
                }
            }
        }

        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
};
