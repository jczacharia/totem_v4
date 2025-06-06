#pragma once

#include <array>

#include "Pattern.h"

class AudioSpectrumPattern final : public Pattern
{
    uint32_t ms_ = 0;
    uint8_t animationPhase = 0;
    bool animationDirectionForward_ = true;

  public:
    static constexpr auto ID = "Audio Spectrum";

    AudioSpectrumPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        animationPhase = random8();
    }

    void render() override
    {
        if (millis() - ms_ > 50)
        {
            ms_ = millis();
            if (animationDirectionForward_)
            {
                animationPhase++;
                if (animationPhase >= 255)
                {
                    animationPhase = 255;
                    animationDirectionForward_ = false;
                }
            }
            else
            {
                animationPhase--;
                if (animationPhase <= 0)
                {
                    animationPhase = 0;
                    animationDirectionForward_ = true;
                }
            }
        }

        for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
        {
            const uint8_t height = MATRIX_HEIGHT * Audio.heights8[x] / 255;
            const uint8_t peakHeight = MATRIX_HEIGHT * Audio.peaks8[x] / 255;

            for (uint8_t y = 0; y < height; ++y)
            {
                const uint8_t norm = 255 * y / (MATRIX_HEIGHT - 1);
                const uint8_t bottom_hue = lerp8by8(HUE_GREEN, HUE_PINK, animationPhase);
                const uint8_t hue = lerp8by8(bottom_hue, HUE_RED, norm);
                Gfx.drawPixel(x, MATRIX_HEIGHT - 1 - y, CHSV(hue, 255, 255));
            }

            if (peakHeight > 0)
            {
                Gfx.drawPixel(x, MATRIX_HEIGHT - 1 - peakHeight, CRGB(255, 255, 255));
            }
        }
    }
};
