#pragma once

class SimpleNoisePattern final : public Pattern
{
    int16_t speedy = 500;
    int16_t speedz = 500;

  public:
    static constexpr auto ID = "Simple Noise";

    SimpleNoisePattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        Noise.noiseX = random16();
        Noise.noiseY = random16();
        Noise.noiseZ = random16();
        Noise.randomize();
        palette = randomPalette();
        speedy = random16(300, 1001) * (random8(0, 2) ? 1 : -1);
        speedz = random16(300, 1001) * (random8(0, 2) ? 1 : -1);
        kaleidoscope = random8(0, 2);
        kaleidoscopeMode = random8(1, Gfx.KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        Noise.noiseY += speedy;
        Noise.noiseZ += speedz;
        Noise.fill();

        for (uint16_t i = 0; i < MATRIX_WIDTH; i++)
        {
            for (uint16_t j = 0; j < MATRIX_HEIGHT; j++)
            {
                const uint8_t color = Noise(i, j);
                GfxBkg(i, j) = ColorFromPalette(palette, color);
            }
        }

        if (kaleidoscope)
        {
            GfxBkg.randomKaleidoscope(kaleidoscopeMode);
            GfxBkg.kaleidoscope1();
        }
    }

    void backgroundPostProcess() override
    {
        GfxBkg.dim(40);
    }
};
