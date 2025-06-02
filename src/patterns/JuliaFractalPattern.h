#pragma once


#include <algorithm>
#include <cmath>

// Helper structure for fractal parameters (Julia Only for this version)
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
private:
    FractalParams currentParams;
    FractalParams previousParams;
    FractalParams targetParams;

    // Morphing Control
    float morphProgress = 1.0f; // 0.0 (start) to 1.0 (end of morph)
    uint8_t morphDurationFrames = 15; // Default frames for one morph cycle (Reduced for faster morphs)
    uint8_t framesSinceBeat = 0;

    // Bounce Control
    float beatZoomFactor = 0.5f; // How much to zoom in on a beat (e.g., 0.5 means zoom to 50% of current)
    float beatZoomEnergySensitivity = 0.8f; // How much beat energy affects the zoom factor

    // Noise Control
    float audioNoiseStrength = 0.02f; // Max strength of noise effect for Julia C params

    // Rotation Control
    float currentAngle_rad = 0.0f;
    float rotationSpeedSensitivity = 0.5f; // Adjust to control spin speed based on energy

    uint32_t hue_ms_global = 0;

    static constexpr int NUM_JULIA_PRESETS = 5;
    const float juliaPresets[NUM_JULIA_PRESETS][2] = {
        {-0.4f, 0.6f}, {0.285f, 0.01f}, {-0.8f, 0.156f}, {-0.70176f, -0.3842f}, {0.355f, 0.355f}
    };

    static float lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    static uint8_t lerp_hue(uint8_t a, uint8_t b, float t)
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

    static float random_float_pm_one()
    {
        return (random8() / 127.5f) - 1.0f;
    }

    void randomizeTargetParameters()
    {
        const uint8_t presetIndex = random8(0, NUM_JULIA_PRESETS);
        targetParams.juliaCX = juliaPresets[presetIndex][0];
        targetParams.juliaCY = juliaPresets[presetIndex][1];
        // Widen the zoom range for more dynamic scaling possibilities
        targetParams.zoom = 0.8f + (random8(0, 220) / 100.0f); // Scale 0.8 to 3.0
        targetParams.maxIterations = 20 + random8(0, 15); // Iterations 20 to 35
        targetParams.hue = random8();
        targetParams.colorSpeed = 0.7f + (random8(0, 100) / 100.0f); // Color speed 0.7 to 1.7
        targetParams.hueCycling = random8(0, 2);
        // Randomize morph duration slightly for variety, keeping it generally faster
        morphDurationFrames = 10 + random8(0, 8); // Morph duration 10-18 frames
    }

public:
    static constexpr auto ID = "JuliaFractal";

    JuliaFractalPattern(): Pattern(ID)
    {
    }

    void start(PatternContext& ctx) override
    {
        hue_ms_global = millis();
        randomizeTargetParameters();
        previousParams = targetParams;
        currentParams = targetParams;
        morphProgress = 1.0f; // Start fully morphed
        framesSinceBeat = morphDurationFrames; // Ensure it's considered "done" initially
        currentAngle_rad = 0.0f; // Reset angle on start
    }

    void render(PatternContext& ctx) override
    {
        if (ctx.audio.isBeat)
        {
            previousParams = currentParams; // Store the actual current state before beat modifications
            randomizeTargetParameters(); // This sets the *target* parameters for the upcoming morph

            // --- Beat Bounce Effect ---
            // Calculate how much to zoom in based on beat energy
            float energyBasedZoomFactor = beatZoomFactor + (ctx.audio.energy8 / 255) * beatZoomEnergySensitivity;
            energyBasedZoomFactor = constrain(energyBasedZoomFactor, 0.1f, 0.9f); // Ensure it's a zoom-in

            // Apply an immediate "bounce" zoom to the *current* parameters, which will be the starting point of the
            // morph We want to zoom *in*, so we decrease the zoom value. The morph will then go from this sharply
            // zoomed-in state to the new targetParams.zoom
            currentParams.zoom *= energyBasedZoomFactor;
            // Ensure zoom doesn't get too extreme from repeated bounces
            currentParams.zoom = max(currentParams.zoom, 0.5f);
            rotationSpeedSensitivity = random8(1, (ctx.audio.totalBeats % 4) + 1) / 10.0f;
            rotationSpeedSensitivity *= random8(0, 2) == 0 ? -1.0f : 1.0f;
            framesSinceBeat = 0;
        }

        if (framesSinceBeat < morphDurationFrames)
        {
            framesSinceBeat++;
        }
        const float t = std::min(1.0f, static_cast<float>(framesSinceBeat) / morphDurationFrames);
        morphProgress = t * (2.0f - t); // Quadratic ease-out: starts fast, slows down

        // Calculate interpolated base parameters for Julia set
        const float baseJuliaCX = lerp(previousParams.juliaCX, targetParams.juliaCX, morphProgress);
        const float baseJuliaCY = lerp(previousParams.juliaCY, targetParams.juliaCY, morphProgress);
        currentParams.zoom = lerp(previousParams.zoom, targetParams.zoom, morphProgress);
        currentParams.maxIterations =
            static_cast<int>(lerp(static_cast<float>(previousParams.maxIterations),
                                  static_cast<float>(targetParams.maxIterations), morphProgress));
        currentParams.hue = lerp_hue(previousParams.hue, targetParams.hue, morphProgress);
        currentParams.colorSpeed = lerp(previousParams.colorSpeed, targetParams.colorSpeed, morphProgress);
        currentParams.hueCycling = (morphProgress < 0.5f) ? previousParams.hueCycling : targetParams.hueCycling;

        // Calculate audio noise factor (using mid-high frequencies for example)
        // You can experiment with different frequency ranges, e.g., bass: rd.heightsRange64(0, MATRIX_WIDTH / 4)
        const float noiseFactor =
            std::min(1.0f, (float)(ctx.audio.energy8 >> 2)); // Normalize, 30 is an estimated typical max for a band

        // Apply noise to the interpolated Julia C parameters
        currentParams.juliaCX = baseJuliaCX + random_float_pm_one() * audioNoiseStrength * noiseFactor;
        currentParams.juliaCY = baseJuliaCY + random_float_pm_one() * audioNoiseStrength * noiseFactor;

        // --- Update Rotation Angle based on Audio Energy ---
        const float energyNormalized = ctx.audio.energy8 / 255; // Normalize energy 0-1
        const float angleIncrement = energyNormalized * rotationSpeedSensitivity;
        currentAngle_rad += angleIncrement;
        if (currentAngle_rad > TWO_PI)
            currentAngle_rad -= TWO_PI; // Keep angle within 0-2PI
        else if (currentAngle_rad < 0)
            currentAngle_rad += TWO_PI;

        const float cosAngle = cos(currentAngle_rad);
        const float sinAngle = sin(currentAngle_rad);

        for (int x_pixel = 0; x_pixel < MATRIX_WIDTH; x_pixel++)
        {
            for (int y_pixel = 0; y_pixel < MATRIX_HEIGHT; y_pixel++)
            {
                // Translate pixel to be relative to center for rotation
                const float translatedX = static_cast<float>(x_pixel) - MATRIX_CENTER_X;
                const float translatedY = static_cast<float>(y_pixel) - MATRIX_CENTER_Y;

                // Apply rotation
                const float rotatedX = translatedX * cosAngle - translatedY * sinAngle;
                const float rotatedY = translatedX * sinAngle + translatedY * cosAngle;

                // Translate back to original coordinate system and map to complex plane
                const float finalX_screen = rotatedX + MATRIX_CENTER_X;
                const float finalY_screen = rotatedY + MATRIX_CENTER_Y;

                // Map pixel to complex plane for initial Z in Julia set
                float zx = (finalX_screen / MATRIX_WIDTH - 0.5f) * (4.0f / currentParams.zoom);
                float zy = (finalY_screen / MATRIX_HEIGHT - 0.5f) * (4.0f / currentParams.zoom);

                int iteration = 0;
                const float c_real = currentParams.juliaCX; // Use noise-affected and morphed Julia C
                const float c_imag = currentParams.juliaCY;

                // Julia set iteration: z_new = z_old^2 + c
                while (zx * zx + zy * zy < 4.0f && iteration < currentParams.maxIterations)
                {
                    const float temp_zx = zx * zx - zy * zy + c_real;
                    zy = 2.0f * zx * zy + c_imag;
                    zx = temp_zx;
                    iteration++;
                }

                if (iteration < currentParams.maxIterations) // Draw if escaped
                {
                    const uint8_t pixelHue = currentParams.hue + static_cast<uint8_t>(iteration * currentParams.
                        colorSpeed);
                    constexpr uint8_t saturation = 255;
                    const uint8_t brightness = map(iteration, 0, currentParams.maxIterations, 60, 255);
                    ctx.leds(x_pixel, y_pixel) += CHSV(pixelHue, saturation, brightness);
                }
            }
        }

        if (currentParams.hueCycling && (millis() - hue_ms_global > 45))
        {
            currentParams.hue++;
            hue_ms_global = millis();
        }

        if (ctx.audio.totalBeats % 8 == 0)
        {
            ctx.kaleidoscope1();
            if (ctx.audio.energy8 > 30)
                ctx.kaleidoscope3();
        }
        else if (ctx.audio.totalBeats % 4 == 0)
        {
            ctx.kaleidoscope3();
            if (ctx.audio.energy8 > 30)
                ctx.kaleidoscope1();
        }
    }
};
