#pragma once

#include <array>
#include <vector>

#include "Matrix.h"

class DNAHelixPattern final : public Pattern
{
    struct Nucleotide
    {
        float x;
        float y;
        uint8_t type; // 0-3 for A,T,G,C
        uint8_t brightness;
    };

    // Pattern parameters
    float helixRotation = 0.0f;
    float helixPhase = 0.0f;
    float helixRadius = MATRIX_WIDTH / 3.0f;
    float helixHeight = MATRIX_HEIGHT / 20.0f;
    uint8_t baseHue = 0;
    uint32_t lastBeatTime = 0;
    uint8_t kaleidoscopeEffect;

    // Mutation effect
    std::vector<std::pair<uint8_t, uint32_t>> mutations; // position, time

    // Wave parameters for organic movement
    float waveOffset = 0.0f;
    float waveAmplitude = 0.0f;

    void drawBasePair(float y, float phase, float energy, bool isMutating, PatternContext& ctx)
    {
        // Calculate x positions for the two strands
        const float angle1 = phase;
        const float angle2 = phase + PI;

        // Add wave distortion based on audio
        const float waveDistort = sinf(y * 0.3f + waveOffset) * waveAmplitude;

        const float x1 = MATRIX_CENTER_X + (helixRadius + waveDistort) * cos(angle1);
        const float x2 = MATRIX_CENTER_X + (helixRadius + waveDistort) * cos(angle2);

        ctx.fillCircle<CRGB>(x1, y, 1, ColorFromPalette(ctx.currentPalette, baseHue, energy));
        ctx.fillCircle<CRGB>(x2, y, 1, ColorFromPalette(ctx.currentPalette, baseHue + 85, energy));
        ctx.drawLine<CRGB>(x1, y, x2, y, CHSV(baseHue + 170, 255, ctx.audio.energy8));
    }

    void drawBackboneConnection(float y1, float phase1, float y2, float phase2, uint8_t side, PatternContext& ctx)
    {
        const float angle1 = (side == 0) ? phase1 : phase1 + PI;
        const float angle2 = (side == 0) ? phase2 : phase2 + PI;

        const float wave1 = sinf(y1 * 0.3f + waveOffset) * waveAmplitude;
        const float wave2 = sinf(y2 * 0.3f + waveOffset) * waveAmplitude;

        const float x1 = MATRIX_CENTER_X + (helixRadius + wave1) * cos(angle1);
        const float x2 = MATRIX_CENTER_X + (helixRadius + wave2) * cos(angle2);

        if (x1 >= 0 && x1 < MATRIX_WIDTH && x2 >= 0 && x2 < MATRIX_WIDTH)
        {
            const CRGB backboneColor = CHSV(baseHue + 64, 150, 100);
            ctx.drawLine<CRGB>(x1, y1, x2, y2, backboneColor);
        }
    }

public:
    static constexpr auto ID = "DNAHelix";

    DNAHelixPattern() : Pattern(ID)
    {
    }

    void start(PatternContext& ctx) override
    {
        helixRotation = random8() / 255.0f * TWO_PI;
        helixPhase = 0.0f;
        baseHue = random8();
        helixRadius = MATRIX_WIDTH / (2.5f + random8(0, 20) / 20.0f);
        helixHeight = MATRIX_HEIGHT / (15.0f + random8(0, 10) / 10.0f);
        mutations.clear();
        waveOffset = 0.0f;
        waveAmplitude = 0.0f;
        kaleidoscopeEffect = random8(1, ctx.KALEIDOSCOPE_COUNT + 1);
    }

    void render(PatternContext& ctx) override
    {
        // Update rotation based on BPM
        const float rotationSpeed = ctx.audio.bpm / 60.0f * 0.05f;
        helixRotation += rotationSpeed;

        // Update wave parameters based on audio energy
        waveOffset += ctx.audio.energy8 * 0.007f;
        waveAmplitude = ctx.audio.energy8 * 0.07f;

        // Add mutations on beats
        if (ctx.audio.isBeat && millis() - lastBeatTime > 100)
        {
            lastBeatTime = millis();
            const uint8_t mutationY = random8(0, MATRIX_HEIGHT);
            mutations.push_back({mutationY, millis()});

            // Limit mutations
            if (mutations.size() > 5)
            {
                mutations.erase(mutations.begin());
            }
        }

        // Remove old mutations
        std::erase_if(mutations, [](const auto& m) { return millis() - m.second > 1000; });

        baseHue++;

        if (ctx.audio.isBeat)
        {
            kaleidoscopeEffect = random8(1, ctx.KALEIDOSCOPE_COUNT + 1);
        }

        // Draw the DNA helix
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
        {
            const float normalizedY = static_cast<float>(y) / MATRIX_HEIGHT;
            const float phase = helixRotation + normalizedY * TWO_PI * 2; // 2 full rotations

            // Check if this position is mutating
            bool isMutating = false;
            for (const auto& [mutY, mutTime] : mutations)
            {
                if (abs(static_cast<int>(y) - static_cast<int>(mutY)) < 3)
                {
                    isMutating = true;
                    break;
                }
            }

            // Use frequency data to modulate the helix
            const uint8_t freqBand = y * 64 / MATRIX_HEIGHT;
            const uint8_t freqEnergy = ctx.audio.heights8[freqBand];

            // Draw base pair
            drawBasePair(y, phase, freqEnergy, isMutating, ctx);

            // Draw backbone connections
            if (y > 0)
            {
                const float prevPhase = helixRotation + ((y - 1) / static_cast<float>(MATRIX_HEIGHT)) * TWO_PI * 2;
                drawBackboneConnection(y - 1, prevPhase, y, phase, 0, ctx); // Left strand
                drawBackboneConnection(y - 1, prevPhase, y, phase, 1, ctx); // Right strand
            }
        }

        if (ctx.audio.totalBeats % 4 == 0)
        {
            ctx.randomKaleidoscope(kaleidoscopeEffect);
            ctx.kaleidoscope2();
        }
    }
};
