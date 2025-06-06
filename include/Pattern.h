#pragma once

#include "MatrixGfx.h"
#include "MatrixNoise.h"
#include "Microphone.h"

class Pattern
{
    std::string id_;

  protected:
    bool linearBlend = false;
    bool kaleidoscope = false;
    uint8_t kaleidoscopeMode = kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    CRGBPalette16 palette = randomPalette();

    explicit Pattern(std::string id)
        : id_(std::move(id))
    {
    }

  public:
    virtual ~Pattern() = default;

    // Foreground graphics
    static MatrixGfx<MATRIX_WIDTH, MATRIX_HEIGHT> Gfx;
    static constexpr size_t KALEIDOSCOPE_COUNT = MatrixGfx<MATRIX_WIDTH, MATRIX_HEIGHT>::KALEIDOSCOPE_COUNT;

    // Background graphics
    static MatrixGfx<MATRIX_WIDTH, MATRIX_HEIGHT> GfxBkg;

    // Gfx Half of matrix canvas
    static MatrixGfx<MATRIX_WIDTH / 2, MATRIX_HEIGHT / 2> GfxCanvasH;

    // Gfx Quarter of matrix canvas
    static MatrixGfx<MATRIX_WIDTH / 4, MATRIX_HEIGHT / 4> GfxCanvasQ;

    FORCE_INLINE_ATTR void clearAllGfx()
    {
        Gfx.clear();
        GfxBkg.clear();
        GfxCanvasH.clear();
        GfxCanvasQ.clear();
    }

    // Noisy data
    static MatrixNoise<MATRIX_WIDTH, MATRIX_HEIGHT> Noise;

    // Musically inclined data
    static AudioContext Audio;

    static uint16_t beatSineOsci[6];       // full 0-65535
    static uint8_t beatSineOsci8[6];       // byte sized 0-255
    static uint8_t beatSineOsciWidth[6];   // matrix width, 0-63 or whatever...
    static uint8_t beatCosineOsciWidth[6]; // matrix width, 0-63 or whatever...
    static uint8_t beatSawOsci8[6];
    static uint8_t beatSawOsciWidth[6];
    static uint8_t beatSquareOsci8[6];
    static uint8_t beatSquareOsciWidth[6];

    virtual void start() = 0;
    virtual void render() = 0;

    virtual void backgroundPostProcess()
    {
    }

    [[nodiscard]] const std::string &getId() const
    {
        return id_;
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

    static void updateBpmOscillators(const uint16_t bpm)
    {

        // oscillators for sine wave forms at variuos rates proportional to the tempo
        // do we need all these? used anywhere?
        // scaled 0-65535
        //
        beatSineOsci[0] = beatsin16(bpm, 0, 65535);
        beatSineOsci[1] = beatsin16(bpm / 2, 0, 65535);
        beatSineOsci[2] = beatsin16(bpm / 4, 0, 65535);
        beatSineOsci[3] = beatsin16(bpm / 8, 0, 65535);
        beatSineOsci[4] = beatsin16(bpm / 16, 0, 65535);
        beatSineOsci[5] = beatsin16(bpm / 32, 0, 65535);

        // scaled 0-255
        //
        beatSineOsci8[0] = beatsin16(bpm, 0, 255);
        beatSineOsci8[1] = beatsin16(bpm / 2, 0, 255);
        beatSineOsci8[2] = beatsin16(bpm / 4, 0, 255);
        beatSineOsci8[3] = beatsin16(bpm / 8, 0, 255);
        beatSineOsci8[4] = beatsin16(bpm / 16, 0, 255);
        beatSineOsci8[5] = beatsin16(bpm / 32, 0, 255);

        // scaled for matrix width (e.g. 0-63)
        //
        beatSineOsciWidth[0] = beatsin16(bpm, 0, MATRIX_HEIGHT - 1);
        beatSineOsciWidth[1] = beatsin16(bpm / 2, 0, MATRIX_HEIGHT - 1);
        beatSineOsciWidth[2] = beatsin16(bpm / 4, 0, MATRIX_HEIGHT - 1);
        beatSineOsciWidth[3] = beatsin16(bpm / 8, 0, MATRIX_HEIGHT - 1);
        beatSineOsciWidth[4] = beatsin16(bpm / 16, 0, MATRIX_HEIGHT - 1);
        beatSineOsciWidth[5] = beatsin16(bpm / 32, 0, MATRIX_HEIGHT - 1);
        beatCosineOsciWidth[0] = beatsin16(bpm, 0, MATRIX_HEIGHT - 1, 0, 16384);
        beatCosineOsciWidth[1] = beatsin16(bpm / 2, 0, MATRIX_HEIGHT - 1, 0, 16384);
        beatCosineOsciWidth[2] = beatsin16(bpm / 4, 0, MATRIX_HEIGHT - 1, 0, 16384);
        beatCosineOsciWidth[3] = beatsin16(bpm / 8, 0, MATRIX_HEIGHT - 1, 0, 16384);
        beatCosineOsciWidth[4] = beatsin16(bpm / 16, 0, MATRIX_HEIGHT - 1, 0, 16384);
        beatCosineOsciWidth[5] = beatsin16(bpm / 32, 0, MATRIX_HEIGHT - 1, 0, 16384);

        // oscillators for saw tooth wave forms, scaled 0-255
        //
        beatSawOsci8[0] = beat8(bpm);
        beatSawOsci8[1] = beat8(bpm / 2);
        beatSawOsci8[2] = beat8(bpm / 4);
        beatSawOsci8[3] = beat8(bpm / 8);
        beatSawOsci8[4] = beat8(bpm / 16);
        beatSawOsci8[5] = beat8(bpm / 32);

        // scaled for matrix width
        //
        beatSawOsciWidth[0] = map8(beatSawOsci8[0], 0, MATRIX_HEIGHT - 1);
        beatSawOsciWidth[1] = map8(beatSawOsci8[1], 0, MATRIX_HEIGHT - 1);
        beatSawOsciWidth[2] = map8(beatSawOsci8[2], 0, MATRIX_HEIGHT - 1);
        beatSawOsciWidth[3] = map8(beatSawOsci8[3], 0, MATRIX_HEIGHT - 1);
        beatSawOsciWidth[4] = map8(beatSawOsci8[4], 0, MATRIX_HEIGHT - 1);
        beatSawOsciWidth[5] = map8(beatSawOsci8[5], 0, MATRIX_HEIGHT - 1);

        // oscillators for square wave forms, scaled 0-255
        //
        beatSquareOsci8[0] = squarewave8(beat8(bpm), 128);
        beatSquareOsci8[1] = squarewave8(beat8(bpm / 2), 128);
        beatSquareOsci8[2] = squarewave8(beat8(bpm / 4), 128);
        beatSquareOsci8[3] = squarewave8(beat8(bpm / 8), 128);
        beatSquareOsci8[4] = squarewave8(beat8(bpm / 16), 128);
        beatSquareOsci8[5] = squarewave8(beat8(bpm / 32), 128);
    }
};

MatrixGfx<MATRIX_WIDTH, MATRIX_HEIGHT> Pattern::Gfx{};
MatrixGfx<MATRIX_WIDTH, MATRIX_HEIGHT> Pattern::GfxBkg{};
MatrixNoise<MATRIX_WIDTH, MATRIX_HEIGHT> Pattern::Noise{};
MatrixGfx<MATRIX_WIDTH / 2, MATRIX_HEIGHT / 2> Pattern::GfxCanvasH;
MatrixGfx<MATRIX_WIDTH / 4, MATRIX_HEIGHT / 4> Pattern::GfxCanvasQ;
AudioContext Pattern::Audio{};
uint16_t Pattern::beatSineOsci[6]{};
uint8_t Pattern::beatSineOsci8[6]{};
uint8_t Pattern::beatSineOsciWidth[6]{};
uint8_t Pattern::beatCosineOsciWidth[6]{};
uint8_t Pattern::beatSawOsci8[6]{};
uint8_t Pattern::beatSawOsciWidth[6]{};
uint8_t Pattern::beatSquareOsci8[6]{};
uint8_t Pattern::beatSquareOsciWidth[6]{};

template <class T>
concept IPattern = std::is_base_of_v<Pattern, T>;
