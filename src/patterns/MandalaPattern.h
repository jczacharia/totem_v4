#pragma once

class MandalaPattern final : public Pattern
{
    int16_t dx = 0;
    int16_t dy = 0;
    int16_t dz = 0;

  public:
    static constexpr auto ID = "Mandala";

    explicit MandalaPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        Noise.randomize();
        dy = random16(2000) - 1000;
        dx = random16(500) - 250;
        dz = random16(500) - 250;
        palette = randomPalette();
        kaleidoscopeMode = random8(1, Gfx.KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                dy = random16(2000) - 1000;
                dx = random16(500) - 250;
                dz = random16(500) - 250;
                Noise.noiseScaleX = random16(10000) + 2000;
                Noise.noiseScaleY = random16(10000) + 2000;
            }
        }

        Noise.noiseY += dy;
        Noise.noiseX += dx;
        Noise.noiseZ += dz;
        Noise.fill();
        Noise.apply(GfxBkg, palette);
        GfxBkg.randomKaleidoscope(kaleidoscopeMode);
        GfxBkg.kaleidoscope1();
    }

    void backgroundPostProcess() override
    {
        GfxBkg.dim(25);
    }
};
