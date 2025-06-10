#pragma once

class RorschachPattern final : public Pattern
{
    int16_t dx = 0;
    int16_t dy = 0;
    int16_t dz = 0;
    uint16_t inkScale = 0;
    uint8_t morphSpeed = 0;
    uint8_t inkThreshold = 120;

    void randomize()
    {
        Noise.randomize();
        dx = random16(400) - 200;
        dy = random16(400) - 200;
        dz = random8();
        inkScale = random16(8000, 15000);
        morphSpeed = random8(100, 200);
        inkThreshold = random8(100, 150);
        Noise.noiseScaleX = inkScale;
        Noise.noiseScaleY = inkScale;
        Noise.noiseSmoothing = morphSpeed;
    }

  public:
    static constexpr auto ID = "Rorschach";

    RorschachPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        randomize();
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        Noise.noiseX += dx;
        Noise.noiseY += dy;
        Noise.noiseZ += dz;
        Noise.fill();

        // Create Rorschach inkblot effect
        // Process only the left half and mirror to right
        for (uint8_t x = 0; x < MATRIX_CENTER_X; x++)
        {
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                const uint8_t noiseValue = Noise(x, y);

                // Create inkblot effect with threshold
                CRGB color{};
                if (noiseValue > inkThreshold)
                {
                    // Create organic blob shapes
                    const uint8_t b = map(Audio.energy8Scaled, 0, 255, 50, 255);
                    color = ColorFromPalette(palette, noiseValue, b);
                }
                else
                {
                    // Background - very dim or black
                    color = ColorFromPalette(palette, noiseValue, 50);
                }

                // Apply to left side
                GfxBkg(x, y) = color;

                // Mirror to right side for symmetry
                GfxBkg(MATRIX_WIDTH - 1 - x, y) = color;
            }
        }

        blur2d(Gfx.data(), Gfx.width(), Gfx.height(), Audio.energy8 / 2);
    }

    void backgroundPostProcess() override
    {
        GfxBkg.dim(25);
    }
};
