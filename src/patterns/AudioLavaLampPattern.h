#pragma once

#include "Pattern.h"

class AudioLavaLampPattern final : public Pattern
{
    // Blob parameters
    static constexpr uint8_t MAX_BLOBS = 12;

    // Blob properties
    uint8_t blob_x[MAX_BLOBS];
    uint8_t blob_y[MAX_BLOBS];
    uint8_t blob_size[MAX_BLOBS];
    int8_t blob_vx[MAX_BLOBS]; // velocity x
    int8_t blob_vy[MAX_BLOBS]; // velocity y
    uint8_t blob_hue[MAX_BLOBS];
    uint8_t blob_life[MAX_BLOBS];
    bool blob_active[MAX_BLOBS];
    uint8_t blob_audio_bin[MAX_BLOBS];

    // Visual parameters
    uint8_t base_hue = 0;
    uint8_t hue_speed = 1;
    uint8_t fade_amount = 245;
    uint8_t gravity_strength = 1;
    uint8_t blob_density = 4;

    // Audio response
    uint8_t audio_sensitivity = 6;
    uint8_t bass_boost = 2;
    uint8_t energy_threshold = 60;
    uint8_t spawn_rate = 3;

    // Pattern variations
    bool rising_mode = true;
    bool merge_blobs = true;
    bool split_blobs = true;
    bool heat_distortion = false;
    uint8_t flow_direction = 0; // 0=up, 1=down, 2=sideways

    // Effects
    bool use_kaleidoscope = false;
    uint8_t kaleidoscope_mode = 1;
    bool glow_effect = true;
    uint8_t temperature_zones = 3;

    void randomize()
    {
        hue_speed = random8(1, 3);
        fade_amount = random8(240, 250);
        gravity_strength = random8(1, 3);
        blob_density = random8(3, 6);
        audio_sensitivity = random8(4, 8);
        bass_boost = random8(1, 4);
        energy_threshold = random8(40, 100);
        spawn_rate = random8(2, 5);

        rising_mode = random8(4) > 0; // 75% chance
        merge_blobs = random8(2);
        split_blobs = random8(2);
        heat_distortion = random8(3) == 0; // 33% chance
        flow_direction = random8(3);

        use_kaleidoscope = random8(4) == 0; // 25% chance
        kaleidoscope_mode = random8(1, KALEIDOSCOPE_COUNT + 1);
        glow_effect = random8(4) > 0; // 75% chance
        temperature_zones = random8(2, 5);
    }

  public:
    static constexpr auto ID = "Audio Lava Lamp";

    AudioLavaLampPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        randomize();
        palette = randomPalette();

        // Initialize blobs
        for (uint8_t i = 0; i < MAX_BLOBS; i++)
        {
            blob_active[i] = false;
            blob_life[i] = 0;
            blob_audio_bin[i] = random8(BINS);
        }
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        // Fade background
        Gfx.dim(fade_amount);

        // Update base hue
        base_hue += hue_speed;

        // Spawn new blobs based on audio
        for (uint8_t i = 0; i < spawn_rate; i++)
        {
            uint8_t audio_index = i * (BINS / spawn_rate);
            if (Audio.heights8[audio_index] > (255 / audio_sensitivity))
            {
                // Find inactive blob to spawn
                for (uint8_t b = 0; b < MAX_BLOBS; b++)
                {
                    if (!blob_active[b])
                    {
                        blob_active[b] = true;
                        blob_audio_bin[b] = audio_index;

                        // Spawn position based on flow direction
                        if (flow_direction == 0) // upward
                        {
                            blob_x[b] = random8(MATRIX_WIDTH);
                            blob_y[b] = MATRIX_HEIGHT - 1;
                            blob_vy[b] = -(1 + (Audio.heights8[audio_index] >> 6));
                            blob_vx[b] = random8(3) - 1;
                        }
                        else if (flow_direction == 1) // downward
                        {
                            blob_x[b] = random8(MATRIX_WIDTH);
                            blob_y[b] = 0;
                            blob_vy[b] = 1 + (Audio.heights8[audio_index] >> 6);
                            blob_vx[b] = random8(3) - 1;
                        }
                        else // sideways
                        {
                            blob_x[b] = 0;
                            blob_y[b] = random8(MATRIX_HEIGHT);
                            blob_vx[b] = 1 + (Audio.heights8[audio_index] >> 6);
                            blob_vy[b] = random8(3) - 1;
                        }

                        blob_size[b] = 2 + (Audio.heights8[audio_index] >> 5);
                        blob_hue[b] = base_hue + (audio_index << 2);
                        blob_life[b] = 255;
                        break;
                    }
                }
            }
        }

        // Update and draw blobs
        for (uint8_t i = 0; i < MAX_BLOBS; i++)
        {
            if (!blob_active[i])
                continue;

            // Update position
            blob_x[i] += blob_vx[i];
            blob_y[i] += blob_vy[i];

            // Apply gravity/buoyancy
            if (rising_mode)
            {
                blob_vy[i] -= gravity_strength; // buoyancy
            }
            else
            {
                blob_vy[i] += gravity_strength; // gravity
            }

            // Audio influence on movement
            uint8_t audio_influence = Audio.heights8[blob_audio_bin[i]] >> 4;
            if (audio_influence > 8)
            {
                blob_vx[i] += random8(3) - 1;
                blob_vy[i] += random8(3) - 1;
            }

            // Limit velocity
            blob_vx[i] = constrain(blob_vx[i], -4, 4);
            blob_vy[i] = constrain(blob_vy[i], -4, 4);

            // Boundary wrapping/bouncing
            if (blob_x[i] >= MATRIX_WIDTH)
            {
                blob_x[i] = 0;
            }
            if (blob_x[i] < 0)
            {
                blob_x[i] = MATRIX_WIDTH - 1;
            }

            // Deactivate if off screen vertically
            if (blob_y[i] >= MATRIX_HEIGHT || blob_y[i] < 0)
            {
                blob_active[i] = false;
                continue;
            }

            // Update blob properties based on audio
            uint8_t audio_energy = Audio.heights8[blob_audio_bin[i]];
            blob_size[i] = 2 + (audio_energy >> 5);

            // Temperature zones affect color
            uint8_t zone = blob_y[i] / (MATRIX_HEIGHT / temperature_zones);
            uint8_t temp_hue = blob_hue[i] + (zone * 32);

            // Draw blob with glow effect
            for (int8_t dx = -blob_size[i]; dx <= blob_size[i]; dx++)
            {
                for (int8_t dy = -blob_size[i]; dy <= blob_size[i]; dy++)
                {
                    uint8_t distance = abs(dx) + abs(dy);
                    if (distance <= blob_size[i])
                    {
                        uint8_t x = blob_x[i] + dx;
                        uint8_t y = blob_y[i] + dy;

                        if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT)
                        {
                            uint8_t brightness = 255 - (distance * 255 / (blob_size[i] + 1));
                            brightness = scale8(brightness, blob_life[i]);

                            // Heat distortion effect
                            if (heat_distortion && distance == blob_size[i])
                            {
                                brightness >>= 1;
                                temp_hue += sin8(millis() >> 2);
                            }

                            Gfx(x, y) += ColorFromPalette(palette, temp_hue, brightness);

                            // Glow effect
                            if (glow_effect && distance == blob_size[i] + 1)
                            {
                                Gfx(x, y) += ColorFromPalette(palette, temp_hue + 16, brightness >> 2);
                            }
                        }
                    }
                }
            }

            // Blob merging
            if (merge_blobs)
            {
                for (uint8_t j = i + 1; j < MAX_BLOBS; j++)
                {
                    if (blob_active[j])
                    {
                        uint8_t dist = abs(blob_x[i] - blob_x[j]) + abs(blob_y[i] - blob_y[j]);
                        if (dist < (blob_size[i] + blob_size[j]))
                        {
                            // Merge blobs
                            blob_size[i] = min(blob_size[i] + 1, 8);
                            blob_life[i] = max(blob_life[i], blob_life[j]);
                            blob_active[j] = false;
                        }
                    }
                }
            }

            // Blob splitting on strong audio
            if (split_blobs && Audio.isBeat && blob_size[i] > 4 && random8(4) == 0)
            {
                for (uint8_t s = 0; s < MAX_BLOBS; s++)
                {
                    if (!blob_active[s])
                    {
                        blob_active[s] = true;
                        blob_x[s] = blob_x[i] + random8(5) - 2;
                        blob_y[s] = blob_y[i] + random8(5) - 2;
                        blob_size[s] = blob_size[i] >> 1;
                        blob_vx[s] = random8(5) - 2;
                        blob_vy[s] = random8(5) - 2;
                        blob_hue[s] = blob_hue[i] + random8(32);
                        blob_life[s] = blob_life[i] >> 1;
                        blob_audio_bin[s] = blob_audio_bin[i];

                        blob_size[i] >>= 1;
                        break;
                    }
                }
            }

            // Fade blob life
            if (blob_life[i] > 5)
            {
                blob_life[i] = scale8(blob_life[i], 250);
            }
            else
            {
                blob_active[i] = false;
            }
        }

        // Apply kaleidoscope effect
        if (use_kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscope_mode);
        }
    }
};
