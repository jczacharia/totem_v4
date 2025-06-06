#pragma once

#include <algorithm>
#include <cmath>

struct FractalParams
{
    float juliaCX = -0.7f;
    float juliaCY = 0.27015f;
    float zoom = 1.5f;
    int maxIterations = 25;
    uint8_t hue = 0;
    float colorSpeed = 1.0f;
    bool hueCycling = true;
};

class JuliaFractalPattern final : public Pattern
{
    FractalParams currentParams;
    FractalParams previousParams;
    FractalParams targetParams;
    float morphProgress = 1.0f;
    uint8_t morphDurationFrames = 15;
    uint8_t framesSinceBeat = 0;
    float beatZoomFactor = 0.5f;
    float beatZoomEnergySensitivity = 0.8f;
    float currentAngle_rad = 0.0f;
    float rotationSpeedSensitivity = 0.5f;
    uint32_t hue_ms_global = 0;
    static constexpr int NUM_JULIA_PRESETS = 5;
    const float juliaPresets[NUM_JULIA_PRESETS][2] =
        {{-0.4f, 0.6f}, {0.285f, 0.01f}, {-0.8f, 0.156f}, {-0.70176f, -0.3842f}, {0.355f, 0.355f}};

    static float lerp(const float a, const float b, const float t)
    {
        return a + t * (b - a);
    }

    static uint8_t lerp_hue(const uint8_t a, const uint8_t b, const float t)
    {
        short diff = b - a;
        if (diff > 128)
        {
            diff -= 256;
        }
        else if (diff < -128)
        {
            diff += 256;
        }
        return static_cast<uint8_t>(a + static_cast<short>(diff * t));
    }

    void randomize()
    {
        const uint8_t presetIndex = random8(0, NUM_JULIA_PRESETS);
        targetParams.juliaCX = juliaPresets[presetIndex][0];
        targetParams.juliaCY = juliaPresets[presetIndex][1];
        targetParams.zoom = 0.8f + (random8(0, 220) / 100.0f);
        targetParams.maxIterations = 20 + random8(0, 15);
        targetParams.hue = random8();
        targetParams.colorSpeed = 0.7f + (random8(0, 100) / 100.0f);
        targetParams.hueCycling = random8(0, 2);
        morphDurationFrames = 10 + random8(0, 8);
    }

  public:
    static constexpr auto ID = "Julia Fractal";

    explicit JuliaFractalPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        randomize();
        hue_ms_global = millis();
        previousParams = targetParams;
        currentParams = targetParams;
        morphProgress = 1.0f;
        framesSinceBeat = morphDurationFrames;
        currentAngle_rad = 0.0f;
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
                previousParams = currentParams;
                float energyBasedZoomFactor = beatZoomFactor + (Audio.energy8 / 255) * beatZoomEnergySensitivity;
                energyBasedZoomFactor = constrain(energyBasedZoomFactor, 0.1f, 0.9f);
                currentParams.zoom *= energyBasedZoomFactor;
                currentParams.zoom = max(currentParams.zoom, 0.5f);
                rotationSpeedSensitivity = random8(1, (Audio.totalBeats % 4) + 1) / 10.0f;
                rotationSpeedSensitivity *= random8(0, 2) == 0 ? -1.0f : 1.0f;
                framesSinceBeat = 0;
            }
        }

        if (framesSinceBeat < morphDurationFrames)
        {
            framesSinceBeat++;
        }

        const float t = std::min(1.0f, static_cast<float>(framesSinceBeat) / morphDurationFrames);
        morphProgress = t * (2.0f - t);

        const float baseJuliaCX = lerp(previousParams.juliaCX, targetParams.juliaCX, morphProgress);
        const float baseJuliaCY = lerp(previousParams.juliaCY, targetParams.juliaCY, morphProgress);
        currentParams.zoom = lerp(previousParams.zoom, targetParams.zoom, morphProgress);
        currentParams.maxIterations = static_cast<int>(lerp(
            static_cast<float>(previousParams.maxIterations),
            static_cast<float>(targetParams.maxIterations),
            morphProgress));
        currentParams.hue = lerp_hue(previousParams.hue, targetParams.hue, morphProgress);
        currentParams.colorSpeed = lerp(previousParams.colorSpeed, targetParams.colorSpeed, morphProgress);
        currentParams.hueCycling = (morphProgress < 0.5f) ? previousParams.hueCycling : targetParams.hueCycling;

        const float noiseFactor = static_cast<float>(Audio.energy8) / 511.0f;
        currentParams.juliaCX = baseJuliaCX + noiseFactor;
        currentParams.juliaCY = baseJuliaCY + noiseFactor;

        const float energyNormalized = static_cast<float>(Audio.energy8) / 255.0f;
        currentAngle_rad += energyNormalized * rotationSpeedSensitivity;
        if (currentAngle_rad > TWO_PI)
        {
            currentAngle_rad -= TWO_PI;
        }
        else if (currentAngle_rad < 0)
        {
            currentAngle_rad += TWO_PI;
        }

        const float cosAngle = cos(currentAngle_rad);
        const float sinAngle = sin(currentAngle_rad);

        for (int x_pixel = 0; x_pixel < MATRIX_WIDTH; x_pixel++)
        {
            for (int y_pixel = 0; y_pixel < MATRIX_HEIGHT; y_pixel++)
            {
                const float translatedX = static_cast<float>(x_pixel) - MATRIX_CENTER_X;
                const float translatedY = static_cast<float>(y_pixel) - MATRIX_CENTER_Y;

                const float rotatedX = translatedX * cosAngle - translatedY * sinAngle;
                const float rotatedY = translatedX * sinAngle + translatedY * cosAngle;

                const float finalX_screen = rotatedX + MATRIX_CENTER_X;
                const float finalY_screen = rotatedY + MATRIX_CENTER_Y;

                float zx = (finalX_screen / MATRIX_WIDTH - 0.5f) * (4.0f / currentParams.zoom);
                float zy = (finalY_screen / MATRIX_HEIGHT - 0.5f) * (4.0f / currentParams.zoom);

                int iter = 0;
                const float c_real = currentParams.juliaCX;
                const float c_imag = currentParams.juliaCY;

                while (zx * zx + zy * zy < 4.0f && iter < currentParams.maxIterations)
                {
                    const float temp_zx = zx * zx - zy * zy + c_real;
                    zy = 2.0f * zx * zy + c_imag;
                    zx = temp_zx;
                    iter++;
                }

                if (iter < currentParams.maxIterations)
                {
                    const uint8_t pixelHue = currentParams.hue + static_cast<uint8_t>(iter * currentParams.colorSpeed);
                    constexpr uint8_t saturation = 255;
                    const uint8_t brightness = map(iter, 0, currentParams.maxIterations, 60, 255);
                    Gfx(x_pixel, y_pixel) += CHSV(pixelHue, saturation, brightness);
                }
            }
        }

        if (currentParams.hueCycling && (millis() - hue_ms_global > 45))
        {
            currentParams.hue++;
            hue_ms_global = millis();
        }

        if (Audio.totalBeats % 8 == 0)
        {
            Gfx.kaleidoscope1();
            if (Audio.energy8 > 200)
                Gfx.kaleidoscope3();
            Gfx.kaleidoscope2();
        }
        else if (Audio.totalBeats % 4 == 0)
        {
            Gfx.kaleidoscope3();
            if (Audio.energy8 > 200)
                Gfx.kaleidoscope1();
            Gfx.kaleidoscope2();
        }
    }
};
