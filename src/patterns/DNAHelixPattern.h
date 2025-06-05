#pragma once

#include <array>

#include "Matrix.h"

class DNAHelixPattern final : public Pattern
{
    float helixRotation = 0.0f;
    float helixRadius = MATRIX_WIDTH / 3.0f;
    float helixHeight = MATRIX_HEIGHT / 20.0f;
    uint8_t baseHue = 0;
    uint32_t lastBeatTime = 0;
    uint8_t kaleidoscopeEffect = 0;
    float waveOffset = 0.0f;
    float waveAmplitude = 0.0f;
    CRGBPalette16 palette = randomPalette();

    void drawBasePair(const float y, const float phase, const float energy, const uint8_t energy8)
    {
        const float angle1 = phase;
        const float angle2 = phase + PI;
        const float waveDistort = sinf(y * 0.3f + waveOffset) * waveAmplitude;
        const float x1 = MATRIX_CENTER_X + (helixRadius + waveDistort) * cos(angle1);
        const float x2 = MATRIX_CENTER_X + (helixRadius + waveDistort) * cos(angle2);
        fillCircle<CRGB>(x1, y, 1, ColorFromPalette(palette, baseHue, energy));
        fillCircle<CRGB>(x2, y, 1, ColorFromPalette(palette, baseHue + 85, energy));
        drawLine<CRGB>(x1, y, x2, y, CHSV(baseHue + 170, 255, energy8));
    }

    void drawBackboneConnection(
        const float y1,
        const float phase1,
        const float y2,
        const float phase2,
        const uint8_t side)
    {
        const float angle1 = (side == 0) ? phase1 : phase1 + PI;
        const float angle2 = (side == 0) ? phase2 : phase2 + PI;
        const float wave1 = sinf(y1 * 0.3f + waveOffset) * waveAmplitude;
        const float wave2 = sinf(y2 * 0.3f + waveOffset) * waveAmplitude;
        const float x1 = MATRIX_CENTER_X + (helixRadius + wave1) * cos(angle1);
        const float x2 = MATRIX_CENTER_X + (helixRadius + wave2) * cos(angle2);
        drawLine<CRGB>(x1, y1, x2, y2, CHSV(baseHue + 64, 150, 100));
    }

  public:
    static constexpr auto ID = "DNAHelix";

    explicit DNAHelixPattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &audio)
        : Pattern(ID, leds, noise, audio)
    {
    }

    void start() override
    {
        helixRotation = random8() / 255.0f * TWO_PI;
        helixRadius = MATRIX_WIDTH / (2.5f + random8(0, 20) / 20.0f);
        helixHeight = MATRIX_HEIGHT / (15.0f + random8(0, 10) / 10.0f);
        waveOffset = 0.0f;
        waveAmplitude = 0.0f;
        kaleidoscopeEffect = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        if (audio.isBeat)
        {
            kaleidoscopeEffect = random8(1, KALEIDOSCOPE_COUNT + 1);
        }

        helixRotation += audio.bpm / 60.0f * 0.05f;
        waveOffset += audio.energy8Peaks * 0.005f;
        waveAmplitude = audio.energy8Peaks * 0.05f;
        baseHue++;

        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
        {
            const float normalizedY = static_cast<float>(y) / MATRIX_HEIGHT;
            const float phase = helixRotation + normalizedY * TWO_PI * 2;
            const uint8_t freqBand = y * 64 / MATRIX_HEIGHT;
            const uint8_t freqEnergy = audio.peaks8[freqBand];
            drawBasePair(y, phase, freqEnergy, audio.energy8Peaks);
            const float prevPhase = helixRotation + ((y - 1) / static_cast<float>(MATRIX_HEIGHT)) * TWO_PI * 2;
            drawBackboneConnection(y - 1, prevPhase, y, phase, 0);
            drawBackboneConnection(y - 1, prevPhase, y, phase, 1);
        }

        if (audio.totalBeats % 4 == 0)
        {
            randomKaleidoscope(kaleidoscopeEffect);
            if (audio.totalBeats % 2 == 0)
            {
                kaleidoscope2();
            }
            else
            {
                kaleidoscope1();
            }
        }
    }
};
