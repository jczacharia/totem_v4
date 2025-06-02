#pragma once

#include "Pattern.h"

class MusicPlaylist final : public Pattern
{
    std::vector<std::string> backgrounds{
        AuroraDropPattern::ID,
        MandalaPattern::ID,
    };
    size_t currentBackgroundIndex = 0;
    std::shared_ptr<Pattern> currentBackground = PatternRegistry::get(backgrounds[currentBackgroundIndex]);

    void nextBackground(PatternContext& ctx)
    {
        currentBackgroundIndex = (currentBackgroundIndex + 1) % backgrounds.size();
        currentBackground = PatternRegistry::get(backgrounds[currentBackgroundIndex]);
        currentBackground->start(ctx);
        Serial.printf("Next background: %s\n", backgrounds[currentBackgroundIndex].c_str());
    }

    std::vector<std::string> patterns{
        MorphingShapesPattern::ID,
        Spectrum2Pattern::ID,
        SpectrumCirclePattern::ID,
        DNAHelixPattern::ID,
        PhyllotaxisFractalPattern::ID,
        RotatingSpectrumPattern::ID,
        AudioTrianglesPattern::ID,
        JuliaFractalPattern::ID,
        AudioSpectrumPattern::ID,
    };
    size_t currentPatternIndex = 0;
    std::shared_ptr<Pattern> currentPattern = PatternRegistry::get(patterns[currentPatternIndex]);

    void nextPattern(PatternContext& ctx)
    {
        currentPatternIndex = (currentPatternIndex + 1) % patterns.size();
        currentPattern = PatternRegistry::get(patterns[currentPatternIndex]);
        currentPattern->start(ctx);
        Serial.printf("Next pattern: %s\n", patterns[currentPatternIndex].c_str());
    }

public:
    static constexpr auto ID = "MusicPlaylist";

    MusicPlaylist() : Pattern(ID)
    {
    }

    void start(PatternContext& ctx) override
    {
    }

    void render(PatternContext& ctx) override
    {
        if (ctx.audio.isBeat)
        {
            if (ctx.audio.totalBeats % 16 == 0)
            {
                nextPattern(ctx);
                ctx.currentPalette = PatternContext::randomPalette();
                ctx.targetPalette = PatternContext::randomPalette();
            }

            if (ctx.audio.totalBeats % 8 == 0)
            {
                nextBackground(ctx);
            }
        }

        if (currentBackground)
        {
            currentBackground->render(ctx);
        }

        ctx.leds.dim(45);

        if (currentPattern)
        {
            currentPattern->render(ctx);
        }
    }
};
