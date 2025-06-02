#pragma once

#include "Geometry.h" // For Point
#include "Matrix.h"
#include <cmath> // For sqrt, cos, sin


class PhyllotaxisFractalPattern final : public Pattern
{
private:
    // Parameters
    float scaleFactor; // Controls the spread of points (c in r = c*sqrt(n))
    uint16_t numPointsCap; // Max points to calculate/draw if energy is maxed out
    uint8_t baseHue;
    uint32_t hue_ms_global;
    bool hueCycling;

    // Beat reaction
    uint8_t beatFlashBrightness;
    uint8_t framesSinceBeatReaction; // Used for both beat flash and morphing
    static constexpr uint8_t BEAT_REACTION_DURATION_FRAMES = 20;

    // Parameters to randomize on beat
    float currentTargetScaleFactor;
    uint8_t currentTargetBaseHue;
    float previousScaleFactor; // For morphing
    uint8_t previousBaseHue; // For morphing
    uint8_t morphDurationFrames;

public:
    static constexpr auto ID = "PhyllotaxisFractal";

    PhyllotaxisFractalPattern(): Pattern(ID)
    {
    }

    void randomizeTargetParameters()
    {
        previousScaleFactor = scaleFactor; // Store current as previous for morph start
        previousBaseHue = baseHue;

        currentTargetScaleFactor = 0.4f + (random8(20, 200) / 100.0f); // e.g., 0.4 to 2.0
        currentTargetBaseHue = random8();
        hueCycling = random8(0, 2);
        morphDurationFrames = 15 + random8(0, 15); // 15-30 frames for morph
    }

    void start(PatternContext& ctx) override
    {
        hue_ms_global = millis();

        // Initial randomization
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
        // Ensure shortest path for hue interpolation
        if (abs(diff) > 128)
        {
            if (b > a)
                diff = -(256 - diff);
            else
                diff = 256 + diff;
        }
        return static_cast<uint8_t>(a + static_cast<short>(diff * t));
    }

    void render(PatternContext& ctx) override
    {
        if (ctx.audio.isBeat)
        {
            randomizeTargetParameters();
            beatFlashBrightness = 255;
            framesSinceBeatReaction = 0;
        }

        // Morphing logic
        float morphProgress = 1.0f;
        if (framesSinceBeatReaction < morphDurationFrames)
        {
            framesSinceBeatReaction++;
            morphProgress = static_cast<float>(framesSinceBeatReaction) / morphDurationFrames;
            morphProgress = constrain(morphProgress, 0.0f, 1.0f);
            // Quadratic ease-out: starts fast, slows down
            morphProgress = morphProgress * (2.0f - morphProgress);
        }

        scaleFactor = lerp_float(previousScaleFactor, currentTargetScaleFactor, morphProgress);
        baseHue = lerp_hue(previousBaseHue, currentTargetBaseHue, morphProgress);

        // Fade beat flash
        if (beatFlashBrightness > 0)
        {
            // A quicker fade for the flash than the morph duration
            beatFlashBrightness = qsub8(beatFlashBrightness, 255 / (BEAT_REACTION_DURATION_FRAMES / 2 + 1));
        }

        ctx.leds.dim(230);

        uint16_t currentMaxPoints = 50 + static_cast<uint16_t>(ctx.audio.energy64f * 3.5f);
        currentMaxPoints = constrain(currentMaxPoints, (uint16_t)30, numPointsCap);

        const float time_based_rotation_rad =
            (float)beat16(5) * (2.0f * M_PI / 65535.0f); // 5 BPM = 12 seconds per full rotation

        for (uint16_t i = 0; i < currentMaxPoints; ++i)
        {
            float angle = i * GOLDEN_ANGLE_RADIANS;
            angle += time_based_rotation_rad;

            const float audioDrivenZoom = 1.0f + ctx.audio.energy64f / 128.0f; // Energy adds up to 50% zoom
            const float radius = scaleFactor * audioDrivenZoom * sqrtf(static_cast<float>(i));

            const float x_base = MATRIX_CENTER_X + radius * cosf(angle);
            const float y_base = MATRIX_CENTER_Y + radius * sinf(angle);

            const uint8_t hue = baseHue + i * (128 / (currentMaxPoints / 2 + 1));
            const uint8_t saturation = 220 + random8(0, 35);

            uint8_t brightness = 80 + static_cast<uint8_t>((1.0f - (radius / (MATRIX_WIDTH / 1.2f))) * 175.0f);
            brightness = constrain(brightness, (uint8_t)20, (uint8_t)255);
            brightness = qadd8(brightness, beatFlashBrightness);
            const uint8_t pointSpecificBrightnessBoost = static_cast<uint8_t>(ctx.audio.heights8[i % MATRIX_WIDTH] / 12
                *
                2.5f);
            brightness = qadd8(brightness, scale8(pointSpecificBrightnessBoost, 100));

            if (x_base >= 0 && x_base < MATRIX_WIDTH && y_base >= 0 && y_base < MATRIX_HEIGHT)
            {
                ctx.leds(static_cast<uint16_t>(x_base), static_cast<uint16_t>(y_base)) +=
                    CHSV(hue, saturation, brightness);
            }

            bool makeFractal = false;
            if (ctx.audio.isBeat && (i % (currentMaxPoints / ((ctx.audio.totalBeats % 4) + 2) + 1) == 0))
            {
                makeFractal = true;
            }
            else if ((i % (currentMaxPoints / 4 + 1) == (framesSinceBeatReaction % (currentMaxPoints / 4 + 1))))
            {
                makeFractal = true;
            }

            if (makeFractal && currentMaxPoints > 10)
            {
                // Only draw if enough points & triggered
                const uint8_t initialBranchLength = ctx.audio.heights8[i % MATRIX_WIDTH] / 14;
                uint8_t fractalHue = hue + 64;

                const Point p_origin = {x_base, y_base};

                // Level 1 branches
                const float branchAngle1_L1 = angle + GOLDEN_ANGLE_RADIANS * 0.35f;
                const float branchAngle2_L1 = angle - GOLDEN_ANGLE_RADIANS * 0.35f;

                Point p1_L1_end;
                p1_L1_end.x = p_origin.x + initialBranchLength * cosf(branchAngle1_L1);
                p1_L1_end.y = p_origin.y + initialBranchLength * sinf(branchAngle1_L1);
                ctx.drawLine<CRGB>(p_origin.x, p_origin.y, p1_L1_end.x, p1_L1_end.y, CHSV(fractalHue, 255, brightness));

                Point p2_L1_end;
                p2_L1_end.x = p_origin.x + initialBranchLength * cosf(branchAngle2_L1);
                p2_L1_end.y = p_origin.y + initialBranchLength * sinf(branchAngle2_L1);
                ctx.drawLine<CRGB>(p_origin.x, p_origin.y, p2_L1_end.x, p2_L1_end.y, CHSV(fractalHue, 255, brightness));

                // Level 2 branches
                const float subBranchLength = initialBranchLength * GOLDEN_RATIO_INV;
                if (subBranchLength >= 0.8f)
                {
                    // Minimum length to draw
                    fractalHue += 32;
                    const uint8_t subBrightness = scale8(brightness, 180); // Dimmer

                    const float branchAngle1_L2a = branchAngle1_L1 + GOLDEN_ANGLE_RADIANS * 0.25f;
                    const float branchAngle1_L2b = branchAngle1_L1 - GOLDEN_ANGLE_RADIANS * 0.25f;
                    Point p1_L2a_end, p1_L2b_end;
                    p1_L2a_end.x = p1_L1_end.x + subBranchLength * cosf(branchAngle1_L2a);
                    p1_L2a_end.y = p1_L1_end.y + subBranchLength * sinf(branchAngle1_L2a);
                    ctx.drawLine<CRGB>(p1_L1_end.x, p1_L1_end.y, p1_L2a_end.x, p1_L2a_end.y,
                                       CHSV(fractalHue, 255, subBrightness));

                    p1_L2b_end.x = p1_L1_end.x + subBranchLength * cosf(branchAngle1_L2b);
                    p1_L2b_end.y = p1_L1_end.y + subBranchLength * sinf(branchAngle1_L2b);
                    ctx.drawLine<CRGB>(p1_L1_end.x, p1_L1_end.y, p1_L2b_end.x, p1_L2b_end.y,
                                       CHSV(fractalHue, 255, subBrightness));

                    // From second L1 branch
                    const float branchAngle2_L2a = branchAngle2_L1 + GOLDEN_ANGLE_RADIANS * 0.25f;
                    const float branchAngle2_L2b = branchAngle2_L1 - GOLDEN_ANGLE_RADIANS * 0.25f;
                    Point p2_L2a_end, p2_L2b_end;
                    p2_L2a_end.x = p2_L1_end.x + subBranchLength * cosf(branchAngle2_L2a);
                    p2_L2a_end.y = p2_L1_end.y + subBranchLength * sinf(branchAngle2_L2a);
                    ctx.drawLine<CRGB>(p2_L1_end.x, p2_L1_end.y, p2_L2a_end.x, p2_L2a_end.y,
                                       CHSV(fractalHue, 255, subBrightness));

                    p2_L2b_end.x = p2_L1_end.x + subBranchLength * cosf(branchAngle2_L2b);
                    p2_L2b_end.y = p2_L1_end.y + subBranchLength * sinf(branchAngle2_L2b);
                    ctx.drawLine<CRGB>(p2_L1_end.x, p2_L1_end.y, p2_L2b_end.x, p2_L2b_end.y,
                                       CHSV(fractalHue, 255, subBrightness));
                }
            }
        }

        if (hueCycling && (millis() - hue_ms_global > 25))
        {
            baseHue++;
            hue_ms_global = millis();
        }

        if (ctx.audio.totalBeats % 4 == 0)
        {
            ctx.kaleidoscope1();
            if (ctx.audio.energy64f > 30)
                ctx.kaleidoscope3();
        }
        else
        {
            ctx.kaleidoscope3();
            if (ctx.audio.energy64f > 30)
                ctx.kaleidoscope1();
        }

        ctx.kaleidoscope2();
    }
};
