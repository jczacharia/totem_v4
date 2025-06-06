#pragma once

class AudioTrianglesPattern final : public Pattern
{
    uint8_t color1 = 0;
    uint8_t color2 = 0;
    uint8_t color3 = 0;
    uint8_t color4 = 0;
    uint8_t color5 = 0;
    uint8_t color6 = 0;

    void randomize()
    {
        color1 = random8();
        color2 = random8();
        color3 = random8();
        color4 = random8();
        color5 = random8();
        color6 = random8();
        kaleidoscopeMode = random8(0, 4);
    }

  public:
    static constexpr auto ID = "Triangles";

    AudioTrianglesPattern()
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
            randomize();
        }

        color1++;
        color2++;
        color3++;
        color4++;
        color5++;
        color6++;

        uint8_t x1 = Audio.heights8[10] >> 3;
        uint8_t y1 = Audio.heights8[20] >> 3;
        uint8_t x2 = Audio.heights8[30] >> 3;
        uint8_t y2 = Audio.heights8[40] >> 3;
        uint8_t x3 = Audio.heights8[50] >> 3;
        uint8_t y3 = Audio.heights8[60] >> 3;

        Gfx.drawLine(x1, y1, x2, y2, ColorFromPalette(palette, color1));
        Gfx.drawLine(x2, y2, x3, y3, ColorFromPalette(palette, color2));
        Gfx.drawLine(x3, y3, x1, y1, ColorFromPalette(palette, color3));

        x1 = Audio.heights8[13] >> 3;
        y1 = Audio.heights8[23] >> 3;
        x2 = Audio.heights8[33] >> 3;
        y2 = Audio.heights8[43] >> 3;
        x3 = Audio.heights8[53] >> 3;
        y3 = Audio.heights8[63] >> 3;

        Gfx.drawLine(x1, y1, x2, y2, ColorFromPalette(palette, color4));
        Gfx.drawLine(x2, y2, x3, y3, ColorFromPalette(palette, color5));
        Gfx.drawLine(x3, y3, x1, y1, ColorFromPalette(palette, color6));

        switch (kaleidoscopeMode)
        {
            case 0:
                Gfx.kaleidoscope3();
                Gfx.kaleidoscope1();
                break;
            case 1:
                Gfx.kaleidoscope4();
                Gfx.kaleidoscope1();
                break;
            case 2: Gfx.kaleidoscope2(); break;
            default:
                Gfx.randomKaleidoscope(random8(1, Gfx.KALEIDOSCOPE_COUNT + 1));
                Gfx.kaleidoscope2();
                break;
        }
    }
};
