#pragma once

#include <Microphone.h>

#include "Matrix.h"

class Pattern
{
    std::string id;

  protected:
    bool linearBlend = false;

    explicit Pattern(std::string id)
        : id(std::move(id))
    {
    }

  public:
    virtual ~Pattern() = default;
    static MatrixLeds leds;
    static MatrixLeds bkgLeds;
    static MatrixNoise noise;
    static AudioContext audio;

    virtual void start() = 0;
    virtual void render() = 0;

    [[nodiscard]] const std::string &getId() const
    {
        return id;
    }

    FORCE_INLINE_ATTR uint8_t beatcos8(
        const accum88 beats_per_minute,
        const uint8_t lowest = 0,
        const uint8_t highest = 255,
        const uint32_t timebase = 0,
        const uint8_t phase_offset = 0)
    {
        const uint8_t beat = beat8(beats_per_minute, timebase);
        const uint8_t beatcos = cos8(beat + phase_offset);
        const uint8_t rangewidth = highest - lowest;
        const uint8_t scaledbeat = scale8(beatcos, rangewidth);
        const uint8_t result = lowest + scaledbeat;
        return result;
    }

    FORCE_INLINE_ATTR uint8_t mapsin8(const uint8_t theta, const uint8_t lowest = 0, const uint8_t highest = 255)
    {
        const uint8_t beatsin = sin8(theta);
        const uint8_t rangewidth = highest - lowest;
        const uint8_t scaledbeat = scale8(beatsin, rangewidth);
        const uint8_t result = lowest + scaledbeat;
        return result;
    }

    FORCE_INLINE_ATTR uint8_t mapcos8(const uint8_t theta, const uint8_t lowest = 0, const uint8_t highest = 255)
    {
        const uint8_t beatcos = cos8(theta);
        const uint8_t rangewidth = highest - lowest;
        const uint8_t scaledbeat = scale8(beatcos, rangewidth);
        const uint8_t result = lowest + scaledbeat;
        return result;
    }

    FORCE_INLINE_ATTR float deg_to_radf(const float degrees)
    {
        return degrees * PI / 180.0f;
    }

    FORCE_INLINE_ATTR CRGBPalette16 randomPalette()
    {
        switch (random8(9))
        {
            case 0: return RainbowColors_p;

            case 1: return OceanColors_p;

            case 2: return CloudColors_p;

            case 3: return ForestColors_p;

            case 4: return PartyColors_p;

            case 5: return CRGBPalette16{CRGB::Purple, CRGB::Blue, CRGB::Purple, CRGB::Blue};

            case 6: return HeatColors_p;

            case 7: return LavaColors_p;

            case 8: return CRGBPalette16{CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White};

            default: return RainbowColors_p;
        }
    }
};

MatrixLeds Pattern::leds{};
MatrixLeds Pattern::bkgLeds{};
MatrixNoise Pattern::noise{};
AudioContext Pattern::audio{};

template <class T>
concept IPattern = std::is_base_of_v<Pattern, T>;
