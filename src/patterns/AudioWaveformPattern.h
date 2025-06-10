#pragma once

#include "Pattern.h"

class AudioWaveformPattern final : public Pattern
{
    // Simple waveform parameters
    uint8_t wave_points[64][2]; // x,y coordinates for wave points
    uint8_t point_count = 48;
    uint8_t base_hue = 0;
    uint8_t hue_speed = 2;
    uint8_t fade_amount = 240;
    uint8_t wave_frequency = 3;
    uint8_t wave_amplitude = 16;
    uint8_t wave_phase = 0;
    uint8_t wave_speed = 4;
    uint8_t wave_thickness = 1;
    bool multiple_waves = false;
    bool vertical_mode = false;

  public:
    static constexpr auto ID = "Audio Waveform";

    AudioWaveformPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Simple randomization
        point_count = random8(32, 56);
        hue_speed = random8(1, 4);
        fade_amount = random8(230, 250);
        wave_frequency = random8(2, 6);
        wave_amplitude = random8(12, 20);
        wave_speed = random8(2, 6);
        wave_thickness = random8(1, 3);
        multiple_waves = random8(3) == 0; // 33% chance
        vertical_mode = random8(2) == 0;  // 50% chance
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat && Audio.totalBeats % 5 == 0)
        {
            wave_frequency = random8(2, 7);
            multiple_waves = random8(4) == 0;
            vertical_mode = random8(3) == 0;
        }

        // Fade background
        Gfx.dim(fade_amount);

        // Update colors and phase
        base_hue += hue_speed;
        wave_phase += wave_speed + (Audio.energy8 >> 5);

        // Modulate wave with audio
        uint8_t audio_amplitude = wave_amplitude + (Audio.energy8 >> 3);
        uint8_t audio_frequency = wave_frequency + (Audio.avgHeights8Range(16, 47) >> 5);

        // Calculate wave points
        if (vertical_mode)
        {
            // Vertical waves (left to right)
            for (uint8_t i = 0; i < point_count; i++)
            {
                uint8_t y = (i * MATRIX_HEIGHT) / point_count;
                uint8_t wave_input = (y * audio_frequency) + wave_phase;
                uint8_t wave_val = sin8(wave_input);
                
                uint8_t x = MATRIX_CENTER_X + ((audio_amplitude * (wave_val - 128)) >> 7);
                
                if (x < MATRIX_WIDTH)
                {
                    wave_points[i][0] = x;
                    wave_points[i][1] = y;
                }
                else
                {
                    wave_points[i][0] = 255; // mark as invalid
                    wave_points[i][1] = 255;
                }
            }
        }
        else
        {
            // Horizontal waves (top to bottom)
            for (uint8_t i = 0; i < point_count; i++)
            {
                uint8_t x = (i * MATRIX_WIDTH) / point_count;
                uint8_t wave_input = (x * audio_frequency) + wave_phase;
                uint8_t wave_val = sin8(wave_input);
                
                uint8_t y = MATRIX_CENTER_Y + ((audio_amplitude * (wave_val - 128)) >> 7);
                
                if (y < MATRIX_HEIGHT)
                {
                    wave_points[i][0] = x;
                    wave_points[i][1] = y;
                }
                else
                {
                    wave_points[i][0] = 255; // mark as invalid
                    wave_points[i][1] = 255;
                }
            }
        }

        // Draw main wave
        for (uint8_t i = 0; i < point_count; i++)
        {
            uint8_t x = wave_points[i][0];
            uint8_t y = wave_points[i][1];

            if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT)
            {
                uint8_t hue = base_hue + (i << 3);
                uint8_t brightness = 200 + (Audio.energy8 >> 2);

                // Draw point with thickness
                for (uint8_t t = 0; t < wave_thickness; t++)
                {
                    if (x + t < MATRIX_WIDTH)
                        Gfx(x + t, y) += ColorFromPalette(palette, hue, brightness);
                    if (y + t < MATRIX_HEIGHT)
                        Gfx(x, y + t) += ColorFromPalette(palette, hue, brightness);
                }

                // Connect to next point for smooth wave
                if (i < point_count - 1 && wave_points[i + 1][0] < MATRIX_WIDTH)
                {
                    uint8_t next_x = wave_points[i + 1][0];
                    uint8_t next_y = wave_points[i + 1][1];
                    
                    if (next_x < MATRIX_WIDTH && next_y < MATRIX_HEIGHT)
                    {
                        Gfx.drawLine(x, y, next_x, next_y, ColorFromPalette(palette, hue, brightness >> 1));
                    }
                }
            }
        }

        // Draw additional waves if enabled
        if (multiple_waves)
        {
            uint8_t phase2 = wave_phase + 85; // different phase
            uint8_t freq2 = audio_frequency + 1;
            
            for (uint8_t i = 0; i < point_count; i += 2) // every other point for performance
            {
                uint8_t x, y;
                
                if (vertical_mode)
                {
                    y = (i * MATRIX_HEIGHT) / point_count;
                    uint8_t wave_val = sin8((y * freq2) + phase2);
                    x = MATRIX_CENTER_X + ((audio_amplitude * (wave_val - 128)) >> 8);
                }
                else
                {
                    x = (i * MATRIX_WIDTH) / point_count;
                    uint8_t wave_val = sin8((x * freq2) + phase2);
                    y = MATRIX_CENTER_Y + ((audio_amplitude * (wave_val - 128)) >> 8);
                }

                if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT)
                {
                    uint8_t hue = base_hue + 128 + (i << 3);
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
                uint8_t spark_x = wave_points[spark_idx][0];
                uint8_t spark_y = wave_points[spark_idx][1];

                if (spark_x < MATRIX_WIDTH && spark_y < MATRIX_HEIGHT)
                {
                    Gfx(spark_x, spark_y) += CHSV(base_hue + random8(64), 255, 255);
                }
            }
        }

        // Beat flash effect
        if (Audio.isBeat && Audio.energy8 > 200)
        {
            // Flash wave peaks
            for (uint8_t i = 0; i < point_count; i += 3)
            {
                uint8_t x = wave_points[i][0];
                uint8_t y = wave_points[i][1];

                if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT)
                {
                    Gfx(x, y) = CRGB::White;
                }
            }
        }
    }
}; 