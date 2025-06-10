#pragma once

#include "Pattern.h"
#include "Util.h"

class AudioWaveInterferencePattern final : public Pattern
{
    struct WaveSource
    {
        uint8_t x, y;        // Position
        uint8_t phase;       // Current phase
        uint8_t frequency;   // Wave frequency
        uint8_t amplitude;   // Wave amplitude
        uint8_t binIndex;    // Which audio bin drives this source
        uint8_t colorOffset; // Color offset for this source
    };

    static constexpr uint8_t MAX_SOURCES = 6;
    WaveSource sources[MAX_SOURCES];
    uint8_t activeSourceCount = 4;
    uint8_t globalPhase = 0;
    uint8_t interferenceThreshold = 80;
    uint8_t waveSpeed = 3;
    bool constructiveMode = true;

  public:
    static constexpr auto ID = "Wave Interference";

    AudioWaveInterferencePattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        activeSourceCount = random8(3, MAX_SOURCES + 1);
        waveSpeed = 1;
        interferenceThreshold = random8(60, 120);
        constructiveMode = random8(2);

        // Initialize wave sources at different positions
        for (uint8_t i = 0; i < activeSourceCount; i++)
        {
            sources[i].x = random8(MATRIX_WIDTH);
            sources[i].y = random8(MATRIX_HEIGHT);
            sources[i].phase = random8();
            sources[i].frequency = random8(8, 20);
            sources[i].amplitude = random8(40, 80);
            sources[i].binIndex = (i * BINS / activeSourceCount) + random8(BINS / activeSourceCount);
            sources[i].colorOffset = i * (255 / activeSourceCount);
        }

        palette = randomPalette();
        kaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        // Beat effects
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 8 == 0)
            {
                // Randomize source positions
                for (uint8_t i = 0; i < activeSourceCount; i++)
                {
                    sources[i].x = random8(MATRIX_WIDTH);
                    sources[i].y = random8(MATRIX_HEIGHT);
                }
                constructiveMode = !constructiveMode;
            }
            if (Audio.totalBeats % 4 == 0)
            {
                kaleidoscope = random8(2);
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
            }
        }

        globalPhase += waveSpeed;

        // Update wave sources based on audio
        for (uint8_t i = 0; i < activeSourceCount; i++)
        {
            sources[i].phase += sources[i].frequency;
            sources[i].amplitude = 30 + (Audio.heights8[sources[i].binIndex] >> 2);

            // Slightly move sources based on audio energy
            if (Audio.energy8 > 100)
            {
                sources[i].x += sin8(globalPhase + i * 42) >> 6; // Small movement
                sources[i].y += cos8(globalPhase + i * 73) >> 6;
                sources[i].x %= MATRIX_WIDTH;
                sources[i].y %= MATRIX_HEIGHT;
            }
        }

        // Clear with slight fade for trails
        Gfx.dim(240);

        // Calculate interference pattern
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                int16_t totalAmplitude = 0;
                uint8_t dominantColor = 0;
                uint8_t maxContribution = 0;

                // Calculate contribution from each wave source
                for (uint8_t i = 0; i < activeSourceCount; i++)
                {
                    // Calculate distance from source
                    int16_t dx = x - sources[i].x;
                    int16_t dy = y - sources[i].y;
                    uint8_t distance = sqrt16(dx * dx + dy * dy);

                    if (distance < 32) // Only calculate if within reasonable range
                    {
                        // Calculate wave amplitude at this point
                        uint8_t wavePhase = sources[i].phase + (distance << 2);
                        int8_t waveValue = sin8(wavePhase) - 128; // Range -128 to 127

                        // Apply distance attenuation
                        if (distance > 0)
                        {
                            waveValue = (waveValue * sources[i].amplitude) / (distance + 8);
                        }
                        else
                        {
                            waveValue = (waveValue * sources[i].amplitude) >> 3;
                        }

                        totalAmplitude += waveValue;

                        // Track dominant color source
                        uint8_t contribution = abs(waveValue);
                        if (contribution > maxContribution)
                        {
                            maxContribution = contribution;
                            dominantColor = sources[i].colorOffset;
                        }
                    }
                }

                // Convert amplitude to brightness and color
                if (abs(totalAmplitude) > interferenceThreshold)
                {
                    uint8_t brightness;
                    uint8_t hue;

                    if (constructiveMode)
                    {
                        // Constructive interference - bright where waves add up
                        brightness = min(255, abs(totalAmplitude) >> 1);
                        hue = dominantColor + (totalAmplitude > 0 ? 0 : 128);
                    }
                    else
                    {
                        // Show interference patterns more clearly
                        brightness = 255 - min(255, abs(totalAmplitude) >> 2);
                        hue = dominantColor + globalPhase + (x + y);
                    }

                    // Add some beat-driven color shifting
                    if (Audio.isBeat && millis() % 200 < 100)
                    {
                        hue += Audio.energy8 >> 2;
                    }

                    Gfx(x, y) += ColorFromPalette(palette, hue, brightness);
                }
            }
        }

        // Draw source points
        for (uint8_t i = 0; i < activeSourceCount; i++)
        {
            uint8_t sourceBrightness = sources[i].amplitude + (Audio.energy8 >> 3);
            if (sourceBrightness > 255)
                sourceBrightness = 255;

            // Draw a small cross at each source
            if (sources[i].x > 0)
                Gfx(sources[i].x - 1, sources[i].y) +=
                    ColorFromPalette(palette, sources[i].colorOffset, sourceBrightness);
            if (sources[i].x < MATRIX_WIDTH - 1)
                Gfx(sources[i].x + 1, sources[i].y) +=
                    ColorFromPalette(palette, sources[i].colorOffset, sourceBrightness);
            if (sources[i].y > 0)
                Gfx(sources[i].x, sources[i].y - 1) +=
                    ColorFromPalette(palette, sources[i].colorOffset, sourceBrightness);
            if (sources[i].y < MATRIX_HEIGHT - 1)
                Gfx(sources[i].x, sources[i].y + 1) +=
                    ColorFromPalette(palette, sources[i].colorOffset, sourceBrightness);

            Gfx(sources[i].x, sources[i].y) += ColorFromPalette(palette, sources[i].colorOffset + 64, 255);
        }

        Gfx.randomKaleidoscope(kaleidoscopeMode);
        if (kaleidoscope)
            Gfx.kaleidoscope1();
        else
            Gfx.kaleidoscope2();

        Gfx.dim(125);
    }
};
