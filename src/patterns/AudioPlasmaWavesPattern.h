#pragma once

#include "Pattern.h"

class AudioPlasmaWavesPattern final : public Pattern
{
    // Wave parameters
    uint8_t waveSpeed1 = 1;
    uint8_t waveSpeed2 = 1;
    uint8_t waveScale1 = 20;
    uint8_t waveScale2 = 15;
    uint16_t waveOffset1 = 0;
    uint16_t waveOffset2 = 0;

    // Audio response
    uint8_t audioInfluence = 128;
    uint8_t bassBoost = 2;
    uint8_t trebleBoost = 3;

    // Visual parameters
    uint8_t colorShift = 0;
    uint8_t colorSpeed = 1;
    bool dualWaves = true;
    bool crossHatch = false;
    uint8_t brightness = 255;
    uint8_t fadeAmount = 240;

    // Pattern variations
    uint8_t waveType = 0; // 0=sine, 1=triangle, 2=square-ish
    bool verticalWaves = false;
    bool diagonalFlow = false;
    uint8_t flowAngle = 0;

    // Beat response
    uint32_t lastBeatTime = 0;
    uint8_t beatWaveExpansion = 0;
    uint8_t beatHueShift = 0;

    // Effects
    bool useKaleidoscope = true;
    uint8_t kaleidoscopeMode = 1;
    bool mirrorWaves = false;
    uint8_t plasmaComplexity = 2;

    void randomize()
    {
        // Randomize wave parameters
        waveSpeed1 = random8(1, 4);
        waveSpeed2 = random8(1, 4);
        waveScale1 = random8(15, 30);
        waveScale2 = random8(10, 25);

        // Audio response
        audioInfluence = random8(100, 200);
        bassBoost = random8(1, 4);
        trebleBoost = random8(1, 4);

        // Visual parameters
        colorSpeed = random8(1, 3);
        dualWaves = random8(3) > 0;   // 66% chance
        crossHatch = random8(3) == 0; // 33% chance
        fadeAmount = random8(220, 250);

        // Pattern variations
        waveType = random8(3);
        verticalWaves = random8(2);
        diagonalFlow = random8(3) == 0; // 33% chance
        flowAngle = random8();

        // Effects
        useKaleidoscope = random8(4) == 0;
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        mirrorWaves = random8(2);
        plasmaComplexity = random8(1, 4);
    }

  public:
    static constexpr auto ID = "Audio Plasma Waves";

    AudioPlasmaWavesPattern()
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
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        // Fade effect
        Gfx.dim(fadeAmount);

        // Update wave offsets based on audio
        waveOffset1 += waveSpeed1 + (Audio.avgHeights8Range(0, 10) >> 6);
        waveOffset2 -= waveSpeed2 + (Audio.avgHeights8Range(54, 63) >> 6);

        // Update color shift
        colorShift += colorSpeed;

        // Beat detection effects
        if (Audio.isBeat)
        {
            lastBeatTime = millis();
            beatWaveExpansion = 20;
            beatHueShift = random8(32, 64);

            // Sometimes change wave direction
            if (random8(8) == 0)
            {
                verticalWaves = !verticalWaves;
            }
        }

        // Decay beat effects
        if (beatWaveExpansion > 0)
        {
            beatWaveExpansion--;
        }
        if (beatHueShift > 0)
        {
            beatHueShift = scale8(beatHueShift, 240);
        }

        // Draw plasma waves
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                // Calculate base plasma value
                uint8_t plasmaValue = 0;

                // First wave component
                uint8_t wave1;
                if (verticalWaves)
                {
                    switch (waveType)
                    {
                        case 0: wave1 = sin8((y * (waveScale1 + beatWaveExpansion)) + (x >> 2) + waveOffset1); break;
                        case 1:
                            wave1 = triwave8((y * (waveScale1 + beatWaveExpansion)) + (x >> 2) + waveOffset1);
                            break;
                        case 2:
                            wave1 = quadwave8((y * (waveScale1 + beatWaveExpansion)) + (x >> 2) + waveOffset1);
                            break;
                        default: wave1 = sin8((y * (waveScale1 + beatWaveExpansion)) + (x >> 2) + waveOffset1); break;
                    }
                }
                else
                {
                    switch (waveType)
                    {
                        case 0: wave1 = sin8((x * (waveScale1 + beatWaveExpansion)) + (y >> 2) + waveOffset1); break;
                        case 1:
                            wave1 = triwave8((x * (waveScale1 + beatWaveExpansion)) + (y >> 2) + waveOffset1);
                            break;
                        case 2:
                            wave1 = quadwave8((x * (waveScale1 + beatWaveExpansion)) + (y >> 2) + waveOffset1);
                            break;
                        default: wave1 = sin8((x * (waveScale1 + beatWaveExpansion)) + (y >> 2) + waveOffset1); break;
                    }
                }

                // Add audio influence to first wave
                uint8_t audioMod1 = Audio.heights8[x] >> 3;
                wave1 = qadd8(wave1, scale8(audioMod1, audioInfluence));

                plasmaValue = wave1;

                // Second wave component (if enabled)
                if (dualWaves)
                {
                    uint8_t wave2;
                    if (crossHatch)
                    {
                        // Perpendicular to first wave
                        if (verticalWaves)
                        {
                            switch (waveType)
                            {
                                case 0: wave2 = sin8((x * waveScale2) + (y >> 2) + waveOffset2); break;
                                case 1: wave2 = triwave8((x * waveScale2) + (y >> 2) + waveOffset2); break;
                                case 2: wave2 = quadwave8((x * waveScale2) + (y >> 2) + waveOffset2); break;
                                default: wave2 = sin8((x * waveScale2) + (y >> 2) + waveOffset2); break;
                            }
                        }
                        else
                        {
                            switch (waveType)
                            {
                                case 0: wave2 = sin8((y * waveScale2) + (x >> 2) + waveOffset2); break;
                                case 1: wave2 = triwave8((y * waveScale2) + (x >> 2) + waveOffset2); break;
                                case 2: wave2 = quadwave8((y * waveScale2) + (x >> 2) + waveOffset2); break;
                                default: wave2 = sin8((y * waveScale2) + (x >> 2) + waveOffset2); break;
                            }
                        }
                    }
                    else
                    {
                        // Same direction but different phase
                        if (verticalWaves)
                        {
                            switch (waveType)
                            {
                                case 0: wave2 = sin8((y * waveScale2) + ((x + 32) >> 2) + waveOffset2); break;
                                case 1: wave2 = triwave8((y * waveScale2) + ((x + 32) >> 2) + waveOffset2); break;
                                case 2: wave2 = quadwave8((y * waveScale2) + ((x + 32) >> 2) + waveOffset2); break;
                                default: wave2 = sin8((y * waveScale2) + ((x + 32) >> 2) + waveOffset2); break;
                            }
                        }
                        else
                        {
                            switch (waveType)
                            {
                                case 0: wave2 = sin8(((x + 32) * waveScale2) + (y >> 2) + waveOffset2); break;
                                case 1: wave2 = triwave8(((x + 32) * waveScale2) + (y >> 2) + waveOffset2); break;
                                case 2: wave2 = quadwave8(((x + 32) * waveScale2) + (y >> 2) + waveOffset2); break;
                                default: wave2 = sin8(((x + 32) * waveScale2) + (y >> 2) + waveOffset2); break;
                            }
                        }
                    }

                    // Add audio influence to second wave
                    uint8_t audioMod2 = Audio.heights8[MATRIX_WIDTH - 1 - x] >> 3;
                    wave2 = qadd8(wave2, scale8(audioMod2, audioInfluence));

                    // Combine waves
                    plasmaValue = qadd8(plasmaValue >> 1, wave2 >> 1);
                }

                // Add diagonal flow component
                if (diagonalFlow)
                {
                    uint8_t diagonalWave = sin8(((x + y) << 2) + flowAngle + waveOffset1);
                    plasmaValue = qadd8(plasmaValue >> 1, diagonalWave >> 1);
                }

                // Add complexity based on distance from center
                if (plasmaComplexity > 1)
                {
                    uint8_t dx = abs(x - MATRIX_CENTER_X);
                    uint8_t dy = abs(y - MATRIX_CENTER_Y);
                    uint8_t distance = (dx + dy) >> 1;
                    uint8_t radialWave = sin8(distance * plasmaComplexity + waveOffset1);
                    plasmaValue = qadd8(plasmaValue >> 1, radialWave >> 2);
                }

                // Apply audio spectrum influence based on position
                uint8_t spectrumInfluence = 0;
                if (y < MATRIX_HEIGHT / 3)
                {
                    // Top third - high frequencies
                    spectrumInfluence = Audio.avgHeights8Range(42, 63) >> 2;
                    spectrumInfluence = scale8(spectrumInfluence, trebleBoost << 6);
                }
                else if (y > 2 * MATRIX_HEIGHT / 3)
                {
                    // Bottom third - low frequencies
                    spectrumInfluence = Audio.avgHeights8Range(0, 21) >> 2;
                    spectrumInfluence = scale8(spectrumInfluence, bassBoost << 6);
                }
                else
                {
                    // Middle third - mid frequencies
                    spectrumInfluence = Audio.avgHeights8Range(21, 42) >> 2;
                }

                plasmaValue = qadd8(plasmaValue, spectrumInfluence);

                // Calculate color
                uint8_t hue = colorShift + plasmaValue + beatHueShift;
                uint8_t saturation = 255;

                // Reduce saturation in low energy areas for depth
                if (plasmaValue < 64)
                {
                    saturation = 192 + plasmaValue;
                }

                // Draw pixel
                Gfx(x, y) = ColorFromPalette(palette, hue, Audio.energy8);

                // Mirror effect
                if (mirrorWaves && plasmaValue > 128)
                {
                    uint8_t mirrorX = MATRIX_WIDTH - 1 - x;
                    Gfx(mirrorX, y) += ColorFromPalette(palette, hue + 64, Audio.energy8 >> 1);
                }
            }
        }

        // Add beat accent dots like AudioDotsSinglePattern
        if (Audio.isBeat && beatWaveExpansion > 10)
        {
            for (uint8_t i = 0; i < 8; i++)
            {
                uint8_t x = beatsin8(i * 32, 0, MATRIX_WIDTH - 1);
                uint8_t y = beatcos8(i * 32, 0, MATRIX_HEIGHT - 1);
                Gfx(x, y) = ColorFromPalette(palette, 255 * x / (MATRIX_WIDTH - 1), Audio.energy8Scaled);
            }
        }

        // Apply kaleidoscope effect
        if (useKaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);

            // Extra effects on beats
            if (Audio.isBeat && Audio.energy8 > 180)
            {
                if (kaleidoscopeMode < 3)
                {
                    Gfx.kaleidoscope1();
                }
            }
        }
    }
};
