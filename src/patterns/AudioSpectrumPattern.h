#pragma once

#include <array>

#include "Pattern.h"

class AudioSpectrumPattern final : public Pattern
{
    uint8_t animationPhase_ = 0;
    bool animationDirectionForward_ = true;
    uint32_t ms = 0;
    bool usePalette = false;

public:
    static constexpr auto ID = "AudioSpectrum";

    AudioSpectrumPattern() : Pattern(ID)
    {
    }

    void start(PatternContext& ctx) override
    {
        animationPhase_ = random(0, 256);
        usePalette = random8(2);
    }

    void render(PatternContext& ctx) override
    {
        if (millis() - ms > 1000)
        {
            ms = millis();
            if (animationDirectionForward_)
            {
                animationPhase_++;
                if (animationPhase_ >= 255)
                {
                    animationPhase_ = 255;
                    animationDirectionForward_ = false;
                }
            }
            else
            {
                animationPhase_--;
                if (animationPhase_ <= 0)
                {
                    animationPhase_ = 0;
                    animationDirectionForward_ = true;
                }
            }
        }

        for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
        {
            const uint8_t height = MATRIX_HEIGHT * ctx.audio.heights8[x] / 255;
            const uint8_t peakHeight = MATRIX_HEIGHT * ctx.audio.peaks8[x] / 255;

            for (uint8_t y = 0; y < height; ++y)
            {
                if (usePalette)
                {
                    const uint8_t norm = 255 * y / (MATRIX_HEIGHT - 1);
                    ctx.drawPixel(x, MATRIX_HEIGHT - 1 - y,
                                  ColorFromPalette(ctx.currentPalette, 255 - norm, 255));
                }
                else
                {
                    const uint8_t norm = 255 * y / (MATRIX_HEIGHT - 1);
                    const uint8_t bottom_hue = lerp8by8(HUE_GREEN, HUE_PINK, animationPhase_);
                    const uint8_t hue = lerp8by8(bottom_hue, HUE_RED, norm);
                    ctx.drawPixel(x, MATRIX_HEIGHT - 1 - y, CHSV(hue, 255, 255));
                }
            }

            if (peakHeight > 0)
            {
                ctx.drawPixel(x, MATRIX_HEIGHT - 1 - peakHeight, CRGB(255, 255, 255));
            }
        }
    }
};
