#pragma once

#include <algorithm>
#include <random>

#include "Pattern.h"

class MusicPlaylist final : public Pattern
{
    std::vector<std::string> backgrounds{
        AuroraDropPattern::ID,
        MandalaPattern::ID,
    };
    size_t bkgIdx = random8(0, backgrounds.size());
    std::shared_ptr<Pattern> currBkg = Registry::get(backgrounds[bkgIdx]);

    void nextBackground()
    {
        bkgIdx = (bkgIdx + 1) % backgrounds.size();
        currBkg = Registry::get(backgrounds[bkgIdx]);
        currBkg->start();
        Serial.printf("Next background: %s\n", backgrounds[bkgIdx].c_str());
    }

    std::vector<std::string> patterns{
        // Audio3dGridPattern::ID,
        // Audio2dGridPattern::ID,
        // AudioClassicSpectrum128Pattern::ID,
        // AudioDotsSinglePattern::ID,
        // Audio2dWavesPattern::ID,
        // AudioCubesPattern::ID,
        // DNAHelixPattern::ID,
        // MorphingShapesPattern::ID,
        // JuliaFractalPattern::ID,
        // Spectrum2Pattern::ID,
        // SpectrumCirclePattern::ID,
        // PhyllotaxisFractalPattern::ID,
        // RotatingSpectrumPattern::ID,
        AudioTrianglesPattern::ID,
        AudioSpectrumPattern::ID,
    };
    // size_t ptnIdx = random8(0, patterns.size());
    size_t ptnIdx = 0;
    std::shared_ptr<Pattern> currPtn = Registry::get(patterns[ptnIdx]);

    void nextPattern()
    {
        ptnIdx = (ptnIdx + 1) % patterns.size();
        currPtn = Registry::get(patterns[ptnIdx]);
        currPtn->start();
        Serial.printf("Next pattern: %s\n", patterns[ptnIdx].c_str());
    }

  public:
    static constexpr auto ID = "MusicPlaylist";

    MusicPlaylist()
        : Pattern(ID)
    {
    }

    void start() override
    {
        std::ranges::shuffle(backgrounds, std::random_device());
        std::ranges::shuffle(patterns, std::random_device());

        currBkg->start();
        currPtn->start();
    }

    void render() override
    {
        if (audio.isBeat)
        {
            if (audio.totalBeats % 16 == 0)
            {
                nextPattern();
                nextBackground();
            }
        }

        currBkg->render();
        currPtn->render();
    }
};
