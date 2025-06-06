#pragma once

#include <algorithm>
#include <random>

#include "Pattern.h"

class MusicPlaylist final : public Pattern
{
    std::vector<std::string> backgrounds{
        AuroraDropPattern::ID,
        LifePattern::ID,
        Mandala2Pattern::ID,
        MandalaPattern::ID,
        MunchPattern::ID,
        PlasmaPattern::ID,
        RorschachPattern::ID,
        SimpleNoisePattern::ID,
    };
    size_t bkgIdx = random8(0, backgrounds.size());
    std::shared_ptr<Pattern> currBkg = Registry::get(backgrounds[bkgIdx]);

    std::vector<uint8_t> waits = {4, 8, 16, 32};
    uint8_t nextPatternWait = waits[random8(0, waits.size())];
    uint8_t nextBackgroundWait = waits[random8(0, waits.size())];
    uint8_t nextForegroundWait = waits[random8(0, waits.size())];

    void nextBackground()
    {
        bkgIdx = (bkgIdx + 1) % backgrounds.size();
        currBkg = Registry::get(backgrounds[bkgIdx]);
        currBkg->start();
        Serial.printf("Next background: %s\n", backgrounds[bkgIdx].c_str());
    }

    std::vector<std::string> patterns{
        Audio8x8SquaresPattern::ID,
        Audio2dGridPattern::ID,
        Audio2dWavesPattern::ID,
        Audio3dGridPattern::ID,
        AudioClassicSpectrum128Pattern::ID,
        AudioCubesPattern::ID,
        AudioDiagonalSpectrumPattern::ID,
        AudioDotsSinglePattern::ID,
        AudioSpectrumPattern::ID,
        AudioTrianglesPattern::ID,
        BigSparkPattern::ID,
        CirclesPattern::ID,
        DNAHelixPattern::ID,
        JuliaFractalPattern::ID,
        MorphingShapesPattern::ID,
        PhyllotaxisFractalPattern::ID,
        RotatingSpectrumPattern::ID,
        Spectrum2Pattern::ID,
        SpectrumCirclePattern::ID,
        SpectrumPeakBarsPattern::ID,
        TorusPattern::ID,
    };
    size_t ptnIdx = random8(0, patterns.size());
    std::shared_ptr<Pattern> currPtn = Registry::get(patterns[ptnIdx]);

    void nextPattern()
    {
        ptnIdx = (ptnIdx + 1) % patterns.size();
        currPtn = Registry::get(patterns[ptnIdx]);
        currPtn->start();
        Serial.printf("Next pattern: %s\n", patterns[ptnIdx].c_str());
    }

  public:
    static constexpr auto ID = "Music Mode";

    MusicPlaylist()
        : Pattern(ID)
    {
    }

    void start() override
    {

        std::ranges::shuffle(backgrounds, std::random_device());
        currBkg->start();

        std::ranges::shuffle(patterns, std::random_device());
        currPtn->start();
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % nextPatternWait == 0)
            {
                nextPatternWait = waits[random8(0, waits.size())];
                nextPattern();
            }

            if (Audio.totalBeats % nextBackgroundWait == 0)
            {
                nextBackgroundWait = waits[random8(0, waits.size())];
                nextBackground();
            }
        }

        currBkg->render();
        currBkg->backgroundPostProcess();
        currPtn->render();
    }
};
