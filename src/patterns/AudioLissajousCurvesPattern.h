#pragma once

#include "Pattern.h"

class AudioLissajousCurvesPattern final : public Pattern
{
    // Simple curve parameters
    uint8_t curve_points[128][2]; // x,y coordinates for curve points
    uint8_t point_count = 64;
    uint8_t base_hue = 0;
    uint8_t hue_speed = 2;
    uint8_t fade_amount = 240;
    uint8_t x_frequency = 3;
    uint8_t y_frequency = 2;
    uint8_t x_phase = 0;
    uint8_t y_phase = 64; // 90 degree offset
    uint8_t amplitude_x = 20;
    uint8_t amplitude_y = 20;
    uint8_t curve_thickness = 1;
    bool multiple_curves = false;
    bool trail_mode = true;

    void randomize()
    {
        // Simple randomization
        point_count = random8(32, 80);
        hue_speed = random8(1, 4);
        fade_amount = random8(230, 250);
        x_frequency = random8(1, 6);
        y_frequency = random8(1, 6);
        x_phase = random8();
        y_phase = random8();
        amplitude_x = random8(15, 25);
        amplitude_y = random8(15, 25);
        curve_thickness = random8(1, 3);
        multiple_curves = random8(3) == 0; // 33% chance
        trail_mode = random8(4) > 0;       // 75% chance
    }

  public:
    static constexpr auto ID = "Audio Lissajous Curves";

    AudioLissajousCurvesPattern()
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
        if (Audio.isBeat && Audio.totalBeats % 4 == 0)
        {
            randomize();
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

        // Update colors and phases
        base_hue += hue_speed;
        x_phase += 1 + (Audio.energy8 >> 6);
        y_phase += 1 + (Audio.energy8 >> 7);

        // Modulate frequencies with audio
        uint8_t audio_mod_x = Audio.avgHeights8Range(0, 15) >> 4;
        uint8_t audio_mod_y = Audio.avgHeights8Range(48, 63) >> 4;
        uint8_t current_x_freq = x_frequency + audio_mod_x;
        uint8_t current_y_freq = y_frequency + audio_mod_y;

        // Modulate amplitude with audio energy
        uint8_t current_amp_x = amplitude_x + (Audio.energy8 >> 4);
        uint8_t current_amp_y = amplitude_y + (Audio.energy8 >> 4);

        // Calculate curve points
        for (uint8_t i = 0; i < point_count; i++)
        {
            uint8_t t = (i * 256) / point_count; // parameter from 0-255

            // Lissajous equations: x = A*sin(a*t + φ), y = B*sin(b*t + ψ)
            uint8_t x_val = sin8((current_x_freq * t) + x_phase);
            uint8_t y_val = sin8((current_y_freq * t) + y_phase);

            // Scale and center
            curve_points[i][0] = MATRIX_CENTER_X + ((current_amp_x * (x_val - 128)) >> 7);
            curve_points[i][1] = MATRIX_CENTER_Y + ((current_amp_y * (y_val - 128)) >> 7);
        }

        // Draw main curve
        for (uint8_t i = 0; i < point_count; i++)
        {
            uint8_t x = curve_points[i][0];
            uint8_t y = curve_points[i][1];

            if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT)
            {
                uint8_t hue = base_hue + (i << 1);
                uint8_t brightness = 200 + (Audio.energy8 >> 2);

                // Draw point with thickness
                for (uint8_t t = 0; t < curve_thickness; t++)
                {
                    if (x + t < MATRIX_WIDTH)
                        Gfx(x + t, y) += ColorFromPalette(palette, hue, brightness);
                    if (y + t < MATRIX_HEIGHT)
                        Gfx(x, y + t) += ColorFromPalette(palette, hue, brightness);
                }

                // Connect to next point for smooth curve
                if (i < point_count - 1)
                {
                    uint8_t next_x = curve_points[i + 1][0];
                    uint8_t next_y = curve_points[i + 1][1];

                    if (next_x < MATRIX_WIDTH && next_y < MATRIX_HEIGHT)
                    {
                        Gfx.drawLine(x, y, next_x, next_y, ColorFromPalette(palette, hue, brightness >> 1));
                    }
                }
            }
        }

        // Draw additional curves if enabled
        if (multiple_curves)
        {
            // Second curve with different frequencies
            uint8_t freq2_x = current_x_freq + 1;
            uint8_t freq2_y = current_y_freq + 2;
            uint8_t phase2_x = x_phase + 85; // different phase
            uint8_t phase2_y = y_phase + 170;

            for (uint8_t i = 0; i < point_count; i += 2) // every other point for performance
            {
                uint8_t t = (i * 256) / point_count;

                uint8_t x_val = sin8((freq2_x * t) + phase2_x);
                uint8_t y_val = sin8((freq2_y * t) + phase2_y);

                uint8_t x = MATRIX_CENTER_X + ((current_amp_x * (x_val - 128)) >> 8);
                uint8_t y = MATRIX_CENTER_Y + ((current_amp_y * (y_val - 128)) >> 8);

                if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT)
                {
                    uint8_t hue = base_hue + 128 + (i << 1);
                    uint8_t brightness = 150 + (Audio.energy8 >> 3);
                    Gfx(x, y) += ColorFromPalette(palette, hue, brightness);
                }
            }
        }

        // Add sparkle effect on strong audio
        if (Audio.energy8 > 180)
        {
            for (uint8_t i = 0; i < 3; i++)
            {
                uint8_t spark_idx = random8(point_count);
                uint8_t spark_x = curve_points[spark_idx][0];
                uint8_t spark_y = curve_points[spark_idx][1];

                if (spark_x < MATRIX_WIDTH && spark_y < MATRIX_HEIGHT)
                {
                    Gfx(spark_x, spark_y) += CHSV(base_hue + random8(64), 255, 255);
                }
            }
        }

        // Beat flash effect
        if (Audio.isBeat && Audio.energy8 > 200)
        {
            // Briefly change curve parameters for dramatic effect
            for (uint8_t i = 0; i < point_count; i += 4)
            {
                uint8_t x = curve_points[i][0];
                uint8_t y = curve_points[i][1];

                if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT)
                {
                    Gfx(x, y) = CRGB::White;
                }
            }
        }
    }
};
