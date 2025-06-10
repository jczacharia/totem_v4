#pragma once

class Mandala2Pattern final : public Pattern
{
    uint32_t dx = 0, dy = 0, dz = 0;

  public:
    static constexpr auto ID = "Mandala 2";

    explicit Mandala2Pattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        Noise.randomize();
        dy = random16(2000) - 1000;
        dx = random16(500) - 250;
        dz = random16(500) - 250;
        Noise.noiseScaleX = random16(10000) + 2000;
        Noise.noiseScaleY = random16(10000) + 2000;
        palette = randomPalette();
        kaleidoscopeMode = random8(1, Gfx.KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        Noise.noiseX += dx;
        Noise.noiseY += dy;
        Noise.noiseZ += dz;
        Noise.fill();
        Noise.apply(GfxBkg, palette, Audio.energy8Scaled);
        blur2d(GfxBkg.data(), GfxBkg.width(), GfxBkg.height(), Audio.energy8 / 2);
        GfxBkg.randomKaleidoscope(kaleidoscopeMode);
        GfxBkg.kaleidoscope1();
    }

    void backgroundPostProcess() override
    {
        GfxBkg.dim(25);
    }
};
