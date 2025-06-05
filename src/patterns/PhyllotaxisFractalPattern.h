#pragma once

#include "Geometry.h"
#include "Matrix.h"
#include <cmath>

class PhyllotaxisFractalPattern final : public Pattern
{
    static constexpr uint8_t BEAT_REACTION_DURATION_FRAMES = 20;
    float scaleFactor = 0;
    uint16_t numPointsCap = 0;
    uint8_t baseHue = 0;
    uint32_t hueMsGlobal = 0;
    bool hueCycling = false;
    uint8_t beatFlashBrightness = 0;
    uint8_t framesSinceBeatReaction = 0;
    float currentTargetScaleFactor = 0;
    uint8_t currentTargetBaseHue = 0;
    float previousScaleFactor = 0;
    uint8_t previousBaseHue = 0;
    uint8_t morphDurationFrames = 0;

  public:
    static constexpr auto ID = "PhyllotaxisFractal";

    explicit PhyllotaxisFractalPattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &audio)
        : Pattern(ID, leds, noise, audio)
    {
    }

    void randomizeTargetParameters()
    {
        previousScaleFactor = scaleFactor;
        previousBaseHue = baseHue;
        currentTargetScaleFactor = 0.4f + (random8(20, 200) / 100.0f);
        currentTargetBaseHue = random8();
        hueCycling = random8(0, 2);
        morphDurationFrames = 15 + random8(0, 15);
    }

    void start() override
    {
        hueMsGlobal = millis();
        scaleFactor = 1.0f + (random8(50, 120) / 100.0f);
        baseHue = random8();
        currentTargetScaleFactor = scaleFactor;
        currentTargetBaseHue = baseHue;
        previousScaleFactor = scaleFactor;
        previousBaseHue = baseHue;
        numPointsCap = 250;
        beatFlashBrightness = 0;
        framesSinceBeatReaction = BEAT_REACTION_DURATION_FRAMES;
        morphDurationFrames = BEAT_REACTION_DURATION_FRAMES;
    }

    static float lerp_float(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    static uint8_t lerp_hue(uint8_t a, uint8_t b, float t)
    {
        short diff = b - a;

        if (abs(diff) > 128)
        {
            if (b > a)
                diff = -(256 - diff);
            else
                diff = 256 + diff;
        }
        return static_cast<uint8_t>(a + static_cast<short>(diff * t));
    }

    void render() override
    {
        if (audio.isBeat)
        {
            if (audio.totalBeats % 4 == 0)
            {
                randomizeTargetParameters();
                beatFlashBrightness = 255;
                framesSinceBeatReaction = 0;
            }
        }

        float morphProgress = 1.0f;
        if (framesSinceBeatReaction < morphDurationFrames)
        {
            framesSinceBeatReaction++;
            morphProgress = static_cast<float>(framesSinceBeatReaction) / morphDurationFrames;
            morphProgress = constrain(morphProgress, 0.0f, 1.0f);
            morphProgress = morphProgress * (2.0f - morphProgress);
        }

        scaleFactor = lerp_float(previousScaleFactor, currentTargetScaleFactor, morphProgress);
        baseHue = lerp_hue(previousBaseHue, currentTargetBaseHue, morphProgress);

        if (beatFlashBrightness > 0)
        {
            beatFlashBrightness = qsub8(beatFlashBrightness, 255 / (BEAT_REACTION_DURATION_FRAMES / 2 + 1));
        }

        leds.dim(230);

        uint16_t currentMaxPoints = 50 + static_cast<uint16_t>(audio.energy64f * 3.5f);
        currentMaxPoints = constrain(currentMaxPoints, (uint16_t)30, numPointsCap);

        const float time_based_rotation_rad = static_cast<float>(beat16(5)) * (2.0f * M_PI / 65535.0f);

        for (uint16_t i = 0; i < currentMaxPoints; ++i)
        {
            float angle = i * GOLDEN_ANGLE_RADIANS;
            angle += time_based_rotation_rad;

            const float audioDrivenZoom = 1.0f + audio.energy64f / 128.0f;
            const float radius = scaleFactor * audioDrivenZoom * sqrtf(static_cast<float>(i));

            const float x_base = MATRIX_CENTER_X + radius * cosf(angle);
            const float y_base = MATRIX_CENTER_Y + radius * sinf(angle);

            const uint8_t hue = baseHue + i * (128 / (currentMaxPoints / 2 + 1));
            const uint8_t saturation = 220 + random8(0, 35);

            uint8_t brightness = 80 + static_cast<uint8_t>((1.0f - (radius / (MATRIX_WIDTH / 1.2f))) * 175.0f);
            brightness = constrain(brightness, (uint8_t)20, (uint8_t)255);
            brightness = qadd8(brightness, beatFlashBrightness);
            const uint8_t pointSpecificBrightnessBoost =
                static_cast<uint8_t>(audio.heights8[i % MATRIX_WIDTH] / 12 * 2.5f);
            brightness = qadd8(brightness, scale8(pointSpecificBrightnessBoost, 100));

            if (x_base >= 0 && x_base < MATRIX_WIDTH && y_base >= 0 && y_base < MATRIX_HEIGHT)
            {
                leds(static_cast<uint16_t>(x_base), static_cast<uint16_t>(y_base)) += CHSV(hue, saturation, brightness);
            }

            bool makeFractal = false;
            if (audio.isBeat && (i % (currentMaxPoints / ((audio.totalBeats % 4) + 2) + 1) == 0))
            {
                makeFractal = true;
            }
            else if ((i % (currentMaxPoints / 4 + 1) == (framesSinceBeatReaction % (currentMaxPoints / 4 + 1))))
            {
                makeFractal = true;
            }

            if (makeFractal && currentMaxPoints > 10)
            {

                const uint8_t initialBranchLength = audio.heights8[i % MATRIX_WIDTH] / 14;
                uint8_t fractalHue = hue + 64;

                const Point p_origin = {x_base, y_base};

                const float branchAngle1_L1 = angle + GOLDEN_ANGLE_RADIANS * 0.35f;
                const float branchAngle2_L1 = angle - GOLDEN_ANGLE_RADIANS * 0.35f;

                Point p1_L1_end;
                p1_L1_end.x = p_origin.x + initialBranchLength * cosf(branchAngle1_L1);
                p1_L1_end.y = p_origin.y + initialBranchLength * sinf(branchAngle1_L1);
                drawLine<CRGB>(p_origin.x, p_origin.y, p1_L1_end.x, p1_L1_end.y, CHSV(fractalHue, 255, brightness));

                Point p2_L1_end;
                p2_L1_end.x = p_origin.x + initialBranchLength * cosf(branchAngle2_L1);
                p2_L1_end.y = p_origin.y + initialBranchLength * sinf(branchAngle2_L1);
                drawLine<CRGB>(p_origin.x, p_origin.y, p2_L1_end.x, p2_L1_end.y, CHSV(fractalHue, 255, brightness));

                if (const float subBranchLength = initialBranchLength * GOLDEN_RATIO_INV; subBranchLength >= 0.8f)
                {

                    fractalHue += 32;
                    const uint8_t subBrightness = scale8(brightness, 180);

                    const float branchAngle1_L2a = branchAngle1_L1 + GOLDEN_ANGLE_RADIANS * 0.25f;
                    const float branchAngle1_L2b = branchAngle1_L1 - GOLDEN_ANGLE_RADIANS * 0.25f;
                    Point p1_L2a_end, p1_L2b_end;
                    p1_L2a_end.x = p1_L1_end.x + subBranchLength * cosf(branchAngle1_L2a);
                    p1_L2a_end.y = p1_L1_end.y + subBranchLength * sinf(branchAngle1_L2a);
                    drawLine<CRGB>(
                        p1_L1_end.x, p1_L1_end.y, p1_L2a_end.x, p1_L2a_end.y, CHSV(fractalHue, 255, subBrightness));

                    p1_L2b_end.x = p1_L1_end.x + subBranchLength * cosf(branchAngle1_L2b);
                    p1_L2b_end.y = p1_L1_end.y + subBranchLength * sinf(branchAngle1_L2b);
                    drawLine<CRGB>(
                        p1_L1_end.x, p1_L1_end.y, p1_L2b_end.x, p1_L2b_end.y, CHSV(fractalHue, 255, subBrightness));

                    const float branchAngle2_L2a = branchAngle2_L1 + GOLDEN_ANGLE_RADIANS * 0.25f;
                    const float branchAngle2_L2b = branchAngle2_L1 - GOLDEN_ANGLE_RADIANS * 0.25f;
                    Point p2_L2a_end, p2_L2b_end;
                    p2_L2a_end.x = p2_L1_end.x + subBranchLength * cosf(branchAngle2_L2a);
                    p2_L2a_end.y = p2_L1_end.y + subBranchLength * sinf(branchAngle2_L2a);
                    drawLine<CRGB>(
                        p2_L1_end.x, p2_L1_end.y, p2_L2a_end.x, p2_L2a_end.y, CHSV(fractalHue, 255, subBrightness));

                    p2_L2b_end.x = p2_L1_end.x + subBranchLength * cosf(branchAngle2_L2b);
                    p2_L2b_end.y = p2_L1_end.y + subBranchLength * sinf(branchAngle2_L2b);
                    drawLine<CRGB>(
                        p2_L1_end.x, p2_L1_end.y, p2_L2b_end.x, p2_L2b_end.y, CHSV(fractalHue, 255, subBrightness));
                }
            }
        }

        if (hueCycling && millis() - hueMsGlobal > 25)
        {
            baseHue++;
            hueMsGlobal = millis();
        }

        if (audio.totalBeats % 4 == 0)
        {
            kaleidoscope1();
            if (audio.energy64f > 30)
            {
                kaleidoscope3();
            }
        }
        else
        {
            kaleidoscope3();
            if (audio.energy64f > 30)
            {
                kaleidoscope1();
            }
        }

        kaleidoscope2();
    }
};
