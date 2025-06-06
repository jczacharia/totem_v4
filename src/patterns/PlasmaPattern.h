#pragma once

class PlasmaPattern final : public Pattern
{
    int time = 0;
    int cycles = 0;
    uint32_t ms = 0;

  public:
    static constexpr auto ID = "Plasma";

    PlasmaPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        kaleidoscope = random(0, 3);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        for (int x = 0; x < GfxBkg.width(); x++)
        {
            for (int y = 0; y < GfxBkg.height(); y++)
            {
                int16_t v = 0;
                uint8_t wibble = sin8(time);
                v += sin16(x * wibble * 2 + time);
                v += cos16(y * (128 - wibble) * 2 + time);
                v += sin16(y * x * cos8(-time) / 2);
                GfxBkg(x, y) = ColorFromPalette(palette, (v >> 8) + 127);
            }
        }

        GfxBkg.randomKaleidoscope(kaleidoscopeMode);
        if (kaleidoscope)
            GfxBkg.kaleidoscope1();
        else
            GfxBkg.kaleidoscope2();

        if (millis() - ms > 50)
        {
            ms = millis();
            time += 1;
            cycles++;

            if (cycles >= 2048)
            {
                time = 0;
                cycles = 0;
            }
        }
    }

    void backgroundPostProcess() override
    {
        GfxBkg.dim(40);
    }
};
