#pragma once

#include "Pattern.h"

class AudioWormholePattern final : public Pattern
{
    // Simple parameters only
    uint8_t rotation_angle = 0;
    uint8_t rotation_speed = 2;
    uint8_t base_hue = 0;
    uint8_t hue_speed = 1;
    uint8_t fade_amount = 230;
    uint8_t ring_count = 8;
    uint8_t center_brightness = 0;
    bool reverse_rotation = false;
    bool use_kaleidoscope = false;
    uint8_t kaleidoscope_mode = 1;

  public:
    static constexpr auto ID = "Audio Wormhole";

    AudioWormholePattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Simple randomization
        rotation_speed = random8(1, 4);
        hue_speed = random8(1, 3);
        fade_amount = random8(220, 240);
        ring_count = random8(6, 12);
        reverse_rotation = random8(2);
        use_kaleidoscope = random8(4) > 0; // 33% chance
        kaleidoscope_mode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat && Audio.totalBeats % 4 == 0)
        {
            reverse_rotation = !reverse_rotation;
            ring_count = random8(6, 12);
        }

        // Fade background
        Gfx.dim(fade_amount);

        // Update rotation
        if (reverse_rotation)
        {
            rotation_angle -= rotation_speed + (Audio.energy8 >> 6);
        }
        else
        {
            rotation_angle += rotation_speed + (Audio.energy8 >> 6);
        }

        // Update colors
        base_hue += hue_speed;

        // Draw concentric rings
        for (uint8_t ring = 0; ring < ring_count; ring++)
        {
            uint8_t radius = ring * 4 + 2;
            uint8_t audio_index = ring * (BINS / ring_count);
            uint8_t audio_height = Audio.heights8[audio_index];

            // Skip ring if no audio
            if (audio_height < 30)
                continue;

            uint8_t ring_brightness = audio_height;
            uint8_t ring_hue = base_hue + (ring * 32);
            uint8_t points_on_ring = 8 + (ring * 2);

            // Draw points around the ring
            for (uint8_t point = 0; point < points_on_ring; point++)
            {
                uint8_t angle = (point * 256 / points_on_ring) + rotation_angle + (ring * 16);

                // Calculate position with perspective effect
                uint8_t perspective_radius = radius + (sin8(angle + (ring * 32)) >> 4);
                uint8_t x =
                    (MATRIX_CENTER_X / 2) + ((perspective_radius * cos8(angle)) >> 7) - (perspective_radius >> 1);
                uint8_t y =
                    (MATRIX_CENTER_Y / 2) + ((perspective_radius * sin8(angle)) >> 7) - (perspective_radius >> 1);

                // Draw point with brightness based on audio
                Gfx(x, y) += ColorFromPalette(palette, ring_hue, ring_brightness);

                // Add spiral effect
                if (ring > 2)
                {
                    uint8_t spiral_x = x + (cos8(angle + 64) >> 6);
                    uint8_t spiral_y = y + (sin8(angle + 64) >> 6);
                    Gfx(spiral_x, spiral_y) += ColorFromPalette(palette, ring_hue + 64, ring_brightness >> 1);
                }
            }
        }

        // Draw center vortex
        center_brightness = Audio.energy8;
        if (center_brightness > 50)
        {
            uint8_t vortex_size = 2 + (center_brightness >> 6);
            for (int8_t dx = -vortex_size; dx <= vortex_size; dx++)
            {
                for (int8_t dy = -vortex_size; dy <= vortex_size; dy++)
                {
                    uint8_t distance = abs(dx) + abs(dy);
                    if (distance <= vortex_size)
                    {
                        uint8_t brightness = center_brightness - (distance * center_brightness / (vortex_size + 1));
                        Gfx(MATRIX_CENTER_X + dx, MATRIX_CENTER_Y + dy) +=
                            ColorFromPalette(palette, base_hue + 128, brightness);
                    }
                }
            }
        }

        // Apply kaleidoscope effect
        if (use_kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscope_mode);
            Gfx.kaleidoscope1();
        }
    }
};
