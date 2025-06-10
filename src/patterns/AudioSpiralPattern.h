#pragma once

#include "Pattern.h"

class AudioSpiralPattern final : public Pattern
{
    // Simple spiral parameters
    uint8_t spiral_points[96][2]; // x,y coordinates for spiral points
    uint8_t point_count = 64;
    uint8_t base_hue = 0;
    uint8_t hue_speed = 3;
    uint8_t fade_amount = 235;
    uint8_t spiral_tightness = 4;
    uint8_t rotation_speed = 2;
    uint8_t current_rotation = 0;
    uint8_t max_radius = 28;
    uint8_t spiral_thickness = 1;
    bool double_spiral = false;
    bool trail_mode = true;

  public:
    static constexpr auto ID = "Audio Spiral";

    AudioSpiralPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Simple randomization
        point_count = random8(48, 80);
        hue_speed = random8(2, 5);
        fade_amount = random8(225, 245);
        spiral_tightness = random8(3, 7);
        rotation_speed = random8(1, 4);
        max_radius = random8(20, 30);
        spiral_thickness = random8(1, 3);
        double_spiral = random8(3) == 0; // 33% chance
        trail_mode = random8(5) > 0;     // 80% chance
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat && Audio.totalBeats % 6 == 0)
        {
            spiral_tightness = random8(3, 8);
            double_spiral = random8(4) == 0;
        }

        // Fade background
        if (trail_mode)
        {
            Gfx.dim(fade_amount);
        }
        else
        {
            Gfx.clear();
        }

        // Update colors and rotation
        base_hue += hue_speed;
        current_rotation += rotation_speed + (Audio.energy8 >> 6);

        // Modulate spiral with audio
        uint8_t audio_radius = max_radius + (Audio.energy8 >> 3);
        uint8_t audio_tightness = spiral_tightness + (Audio.avgHeights8Range(0, 31) >> 5);

        // Calculate spiral points using polar coordinates
        for (uint8_t i = 0; i < point_count; i++)
        {
            uint8_t t = (i * 255) / point_count; // parameter from 0-255
            
            // Spiral equation: r = a * θ, where θ increases with t
            uint8_t angle = (t * audio_tightness) + current_rotation;
            uint8_t radius = (t * audio_radius) >> 8;

            // Convert to cartesian coordinates
            int16_t x_offset = (radius * cos8(angle)) >> 7;
            int16_t y_offset = (radius * sin8(angle)) >> 7;
            
            int16_t x = MATRIX_CENTER_X + x_offset - 128;
            int16_t y = MATRIX_CENTER_Y + y_offset - 128;

            // Bounds check and store
            if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
            {
                spiral_points[i][0] = x;
                spiral_points[i][1] = y;
            }
            else
            {
                spiral_points[i][0] = 255; // mark as invalid
                spiral_points[i][1] = 255;
            }
        }

        // Draw main spiral
        for (uint8_t i = 0; i < point_count; i++)
        {
            uint8_t x = spiral_points[i][0];
            uint8_t y = spiral_points[i][1];

            if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT)
            {
                uint8_t hue = base_hue + (i << 2);
                uint8_t brightness = 180 + (Audio.energy8 >> 2);

                // Draw point with thickness
                for (uint8_t t = 0; t < spiral_thickness; t++)
                {
                    if (x + t < MATRIX_WIDTH)
                        Gfx(x + t, y) += ColorFromPalette(palette, hue, brightness);
                    if (y + t < MATRIX_HEIGHT)
                        Gfx(x, y + t) += ColorFromPalette(palette, hue, brightness);
                }

                // Connect to next point for smooth spiral
                if (i < point_count - 1 && spiral_points[i + 1][0] < MATRIX_WIDTH)
                {
                    uint8_t next_x = spiral_points[i + 1][0];
                    uint8_t next_y = spiral_points[i + 1][1];
                    
                    if (next_x < MATRIX_WIDTH && next_y < MATRIX_HEIGHT)
                    {
                        Gfx.drawLine(x, y, next_x, next_y, ColorFromPalette(palette, hue, brightness >> 1));
                    }
                }
            }
        }

        // Draw counter-rotating spiral if enabled
        if (double_spiral)
        {
            uint8_t counter_rotation = 255 - current_rotation;
            
            for (uint8_t i = 0; i < point_count; i += 2) // every other point for performance
            {
                uint8_t t = (i * 255) / point_count;
                uint8_t angle = (t * audio_tightness) + counter_rotation;
                uint8_t radius = (t * audio_radius) >> 8;

                int16_t x_offset = (radius * cos8(angle)) >> 7;
                int16_t y_offset = (radius * sin8(angle)) >> 7;
                
                int16_t x = MATRIX_CENTER_X + x_offset - 128;
                int16_t y = MATRIX_CENTER_Y + y_offset - 128;

                if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                {
                    uint8_t hue = base_hue + 128 + (i << 2);
                    uint8_t brightness = 150 + (Audio.energy8 >> 3);
                    Gfx(x, y) += ColorFromPalette(palette, hue, brightness);
                }
            }
        }

        // Add sparkle effect on strong audio
        if (Audio.energy8 > 170)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                uint8_t spark_idx = random8(point_count);
                uint8_t spark_x = spiral_points[spark_idx][0];
                uint8_t spark_y = spiral_points[spark_idx][1];

                if (spark_x < MATRIX_WIDTH && spark_y < MATRIX_HEIGHT)
                {
                    Gfx(spark_x, spark_y) += CHSV(base_hue + random8(64), 255, 255);
                }
            }
        }

        // Beat pulse effect
        if (Audio.isBeat && Audio.energy8 > 190)
        {
            // Highlight spiral center
            for (uint8_t r = 0; r < 3; r++)
            {
                for (uint8_t a = 0; a < 255; a += 32)
                {
                    int16_t x = MATRIX_CENTER_X + ((r * cos8(a)) >> 7) - 128;
                    int16_t y = MATRIX_CENTER_Y + ((r * sin8(a)) >> 7) - 128;
                    
                    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                    {
                        Gfx(x, y) = CRGB::White;
                    }
                }
            }
        }
    }
}; 