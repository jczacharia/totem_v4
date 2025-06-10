#pragma once

class AudioBreathingMandalaPattern final : public Pattern
{
    // Noise movement parameters
    int16_t dx = 0;
    int16_t dy = 0;
    int16_t dz = 0;

    // Breathing effect parameters
    uint16_t baseScale = 6000;
    uint16_t breatheAmplitude = 4000;
    uint8_t breatheSpeed = 2;
    uint8_t breathePhase = 0;

    // Visual parameters
    uint8_t energySmoothing = 180;
    uint8_t lastEnergyLevel = 0;
    uint8_t morphSpeed = 150;
    uint8_t colorShift = 0;
    uint8_t colorShiftSpeed = 1;

    // Audio reactivity
    uint8_t audioSensitivity = 4;
    bool useAudioBreathe = true;
    bool useSymmetry = false;

    void randomize()
    {
        // Randomize noise movement
        dx = random16(600) - 300;
        dy = random16(600) - 300;
        dz = random8(50, 200);

        // Randomize breathing parameters
        baseScale = random16(4000, 10000);
        breatheAmplitude = random16(2000, 6000);
        breatheSpeed = random8(1, 4);

        // Randomize visual parameters
        morphSpeed = random8(120, 200);
        colorShiftSpeed = random8(1, 3);
        audioSensitivity = random8(3, 7);
        useAudioBreathe = random8(3) > 0; // 66% chance
        useSymmetry = random8(3) == 0;    // 33% chance

        // Update noise parameters
        Noise.noiseSmoothing = morphSpeed;
        kaleidoscopeMode = random8(1, Gfx.KALEIDOSCOPE_COUNT + 1);
    }

  public:
    static constexpr auto ID = "Audio Breathing Mandala";

    AudioBreathingMandalaPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        Noise.randomize();
        randomize();
        palette = randomPalette();
    }

    void render() override
    {
        // Beat detection effects
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        // Smooth audio energy for breathing
        uint8_t currentEnergy = Audio.energy8 >> audioSensitivity;
        lastEnergyLevel = ((lastEnergyLevel * energySmoothing) + (currentEnergy * (256 - energySmoothing))) >> 8;

        // Calculate breathing scale
        uint16_t breatheScale = baseScale;

        if (useAudioBreathe)
        {
            // Audio-driven breathing - energy controls the breathing intensity
            breatheScale += ((lastEnergyLevel * breatheAmplitude) >> 8);
        }
        else
        {
            // Rhythmic breathing using sine wave
            breathePhase += breatheSpeed;
            uint8_t breatheSine = sin8(breathePhase);
            breatheScale += ((breatheSine * breatheAmplitude) >> 8);
        }

        // Update noise movement
        Noise.noiseX += dx;
        Noise.noiseY += dy;
        Noise.noiseZ += dz;

        // Apply breathing to noise scales
        Noise.noiseScaleX = breatheScale;
        Noise.noiseScaleY = breatheScale;

        // Generate noise
        Noise.fill();

        // Color shifting for dynamic palette cycling
        colorShift += colorShiftSpeed;

        if (useSymmetry)
        {
            // Create symmetrical mandala (like RorschachPattern)
            for (uint8_t x = 0; x < MATRIX_CENTER_X; x++)
            {
                for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
                {
                    uint8_t noiseValue = Noise(x, y);
                    uint8_t brightness = 100 + ((Audio.energy8Scaled * 155) >> 8);

                    CRGB color = ColorFromPalette(palette, noiseValue + colorShift, brightness);

                    // Apply to left side
                    GfxBkg(x, y) = color;
                    // Mirror to right side
                    GfxBkg(MATRIX_WIDTH - 1 - x, y) = color;
                }
            }
        }
        else
        {
            // Standard noise application
            uint8_t brightness = 80 + ((Audio.energy8Scaled * 175) >> 8);
            Noise.apply(GfxBkg, palette, brightness);
        }

        // Audio-reactive blur for breathing softness
        uint8_t blurAmount = (lastEnergyLevel >> 3) + 1; // 1-32 blur range
        blur2d(GfxBkg.data(), GfxBkg.width(), GfxBkg.height(), blurAmount);

        // Apply kaleidoscope effects for mandala structure
        GfxBkg.randomKaleidoscope(kaleidoscopeMode);

        // Additional kaleidoscope on strong beats
        if (Audio.isBeat && Audio.energy8 > 180)
        {
            if (random8(2))
            {
                GfxBkg.kaleidoscope1();
            }
            else
            {
                GfxBkg.kaleidoscope2();
            }
        }
        else
        {
            // Gentle kaleidoscope for continuous mandala effect
            GfxBkg.kaleidoscope1();
        }
    }

    void backgroundPostProcess() override
    {
        // Gentle fading to create flowing trails
        GfxBkg.dim(25);
    }
};
