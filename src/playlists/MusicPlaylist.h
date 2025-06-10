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
        PlasmaPattern::ID,
        RorschachPattern::ID,
        SimpleNoisePattern::ID,
        AudioBreathingMandalaPattern::ID,
    };
    size_t bkgIdx = random8(0, backgrounds.size());
    std::shared_ptr<Pattern> currBkg = Registry::get(backgrounds[bkgIdx]);

    std::vector<uint8_t> waits = {8, 16, 32};
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
        Audio2dGridPattern::ID,
        Audio2dWavesPattern::ID,
        Audio3dGridPattern::ID,
        Audio8x8SquaresPattern::ID,
        AudioAtomicPattern::ID,
        AudioBouncingDotsPattern::ID,
        AudioBreathingRingsPattern::ID,
        AudioClassicSpectrum128Pattern::ID,
        AudioCometPattern::ID,
        AudioCrystalLatticePattern::ID,
        AudioCubesPattern::ID,
        AudioDiagonalSpectrumPattern::ID,
        AudioDotsSinglePattern::ID,
        AudioGeometricFlowerPattern::ID,
        AudioHurricanePattern::ID,
        AudioLavaLampPattern::ID,
        AudioLissajousCurvesPattern::ID,
        AudioMandalaLayersPattern::ID,
        AudioMandelbrotPattern::ID,
        AudioMeteorShowerPattern::ID,
        AudioNeuralNetworkPattern::ID,
        AudioOrbitPattern::ID,
        AudioParticleFlowPattern::ID,
        AudioPlasmaWavesPattern::ID,
        AudioPlatonicSolidsPattern::ID,
        AudioRainMatrixPattern::ID,
        AudioRipplesPattern::ID,
        AudioRotatingBarsPattern::ID,
        AudioSimpleDiagonalPattern::ID,
        AudioSimpleTrianglesPattern::ID,
        AudioSpaceDebrisPattern::ID,
        AudioSpectrumDotsPattern::ID,
        AudioSpectrumPattern::ID,
        AudioSpiralGalaxyPattern::ID,
        AudioSpiralsPattern::ID,
        AudioStarfieldPattern::ID,
        AudioTessellationPattern::ID,
        AudioTrianglesPattern::ID,
        AudioTunnelPattern::ID,
        AudioWaveInterferencePattern::ID,
        AudioWaveformPattern::ID,
        AudioWormholePattern::ID,
        CirclesPattern::ID,
        DNAHelixPattern::ID,
        JuliaFractalPattern::ID,
        MorphingShapesPattern::ID,
        PhyllotaxisFractalPattern::ID,
        RotatingSpectrumPattern::ID,
        SpectrumCirclePattern::ID,
        SpectrumPeakBarsPattern::ID,
        TorusPattern::ID,
    };
    size_t ptnIdx = random8(0, patterns.size());
    // size_t ptnIdx = 0;
    std::shared_ptr<Pattern> currPtn = Registry::get(patterns[ptnIdx]);

    void nextPattern()
    {
        ptnIdx = (ptnIdx + 1) % patterns.size();
        currPtn = Registry::get(patterns[ptnIdx]);
        currPtn->start();
        Serial.printf("Next pattern: %s\n", patterns[ptnIdx].c_str());
    }

    void prevPattern()
    {
        ptnIdx = (ptnIdx - 1) % patterns.size();
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
