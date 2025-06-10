#pragma once

class Spectrum2Pattern final : public Pattern
{
    uint8_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    bool mirror = false;
    uint8_t colorSpread = 0;
    uint8_t kaleidoscopeMode = 0;
    CRGBPalette16 palette = randomPalette();

    void randomize()
    {
        colorSpread = 7;
        kaleidoscope = random(0, 2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    }

  public:
    static constexpr auto ID = "Spectrum 2";

    explicit Spectrum2Pattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        randomize();
        palette = randomPalette();
    };

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        for (uint i = 0; i < MATRIX_WIDTH / 2; i++)
        {
            const uint8_t data = Audio.heights8[2 * i] >> 2;
            x1 = i;
            x2 = i;
            y1 = MATRIX_HEIGHT - 1;
            y2 = MATRIX_HEIGHT - data;

            const uint8_t space = 64 - (y1 - y2);
            y1 = y1 - (space / 2);
            y2 = y2 - (space / 2);

            Gfx.drawLine(x1, y1, x2, y2, ColorFromPalette(palette, i * colorSpread, Audio.energy8Scaled));
            Gfx.drawLine(
                MATRIX_WIDTH - x1 - 1,
                y1,
                MATRIX_WIDTH - x2 - 1,
                y2,
                ColorFromPalette(palette, i * colorSpread, Audio.energy8Scaled));
        }
    }
};
