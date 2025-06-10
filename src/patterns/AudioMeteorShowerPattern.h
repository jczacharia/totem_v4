#pragma once

#include "Pattern.h"

class AudioMeteorShowerPattern final : public Pattern
{
    // Parameters for the effect
    static constexpr uint8_t MAX_METEORS = BINS;

    // Meteor properties
    int16_t meteor_y_fixed[MAX_METEORS]; // y position, 8.8 fixed point. can be negative to start off-screen
    uint8_t meteor_x[MAX_METEORS];       // x position
    uint16_t meteor_speed[MAX_METEORS];  // falling speed, 8.8 fixed point
    uint8_t meteor_len[MAX_METEORS];     // length of tail
    uint8_t meteor_hue[MAX_METEORS];     // color
    bool meteor_active[MAX_METEORS];     // is it currently on screen

    // General effect properties
    uint8_t hue_offset = 0;
    uint8_t fade_amount = 220;
    bool enable_kaleidoscope = false;
    uint8_t kaleidoscope_mode_ = 1;
    bool trails = true;

    void randomize()
    {
        fade_amount = random8(200, 240);
        enable_kaleidoscope = random8(3) > 0; // 66% chance
        kaleidoscope_mode_ = random8(1, KALEIDOSCOPE_COUNT + 1);
        trails = random8(2);
    }

  public:
    static constexpr auto ID = "Audio Meteor Shower";

    AudioMeteorShowerPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        palette = randomPalette();

        // Initialize all meteors to inactive
        for (uint8_t i = 0; i < MAX_METEORS; ++i)
        {
            meteor_active[i] = false;
        }
    }

    void render() override
    {
        if (trails)
        {
            Gfx.dim(fade_amount);
        }
        else
        {
            Gfx.clear();
        }

        hue_offset++;

        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }

            // Spawn a burst of meteors on beat
            uint8_t meteorsToSpawn = 1 + (Audio.energy8 >> 4);
            for (uint8_t k = 0; k < meteorsToSpawn; ++k)
            {
                // Find an inactive meteor to spawn
                for (uint8_t i = 0; i < MAX_METEORS; ++i)
                {
                    if (!meteor_active[i])
                    {
                        meteor_active[i] = true;
                        meteor_x[i] = random8(MATRIX_WIDTH);
                        // Start up to 16 pixels off-screen, using 8.8 fixed point
                        meteor_y_fixed[i] = -(int16_t)(random8(1, 16) << 8);
                        // Speed in pixels per frame (8.8 fixed point)
                        meteor_speed[i] = Audio.heights8[i]; // Slower to faster meteors
                        meteor_len[i] = random8(5, 15);
                        meteor_hue[i] = hue_offset + random8(64);
                        break; // Found a slot, stop searching
                    }
                }
            }
        }

        // update and draw meteors
        for (uint8_t i = 0; i < MAX_METEORS; ++i)
        {
            if (meteor_active[i])
            {
                meteor_y_fixed[i] += meteor_speed[i];
                int16_t y = meteor_y_fixed[i] >> 8;

                // Draw meteor head and trail
                for (uint8_t j = 0; j < meteor_len[i]; j++)
                {
                    int16_t current_y = y - j;
                    if (current_y >= 0 && current_y < MATRIX_HEIGHT)
                    {
                        // Fade out the tail
                        uint8_t brightness = 255 - (j * 255 / meteor_len[i]);
                        Gfx.drawPixel(meteor_x[i], current_y, ColorFromPalette(palette, meteor_hue[i], brightness));
                    }
                }

                // Deactivate if the whole meteor is off-screen
                if (y - meteor_len[i] >= MATRIX_HEIGHT)
                {
                    meteor_active[i] = false;
                }
            }
        }

        if (enable_kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscope_mode_);
        }
    }
};
