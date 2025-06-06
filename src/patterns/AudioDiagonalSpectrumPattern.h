#pragma once

class AudioDiagonalSpectrumPattern final : public Pattern
{
  public:
    static constexpr auto ID = "Diagonal Spectrum";

    AudioDiagonalSpectrumPattern()
        : Pattern(ID)
    {
    }

    bool cycleColors = false;
    uint8_t color1 = 0;
    uint8_t color2 = 64;
    uint8_t color3 = 128;
    uint8_t color4 = 192;
    uint8_t brightness = 192;
    bool k1or2 = false;
    uint8_t mirrorMode = 0; // 0 = top left / bottom right, top right / bottom left, 2= both

    void randomize()
    {
        cycleColors = random(0, 3);
        k1or2 = random(0, 2);
        kaleidoscopeMode = random8(1, Gfx.KALEIDOSCOPE_COUNT + 1);
        mirrorMode = random(0, 3);
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

        if (cycleColors)
        {
            color1++;
            color2++;
            color3++;
            color4++;
            if (brightness > 250)
                brightness = 63;
            brightness++;
        }
        else
        {
            color1 = 0;
            color2 = 64;
            color3 = 128;
            color4 = 192;
        }

        int tx1, ty1;
        int x0, y0, x1, y1;
        byte height;

        if (mirrorMode == 0 || mirrorMode == 2)
        {
            for (int i = 0; i < Gfx.width(); i++)
            {
                height = 4 * Audio.heights8[i] / Gfx.width();
                if (height > 0)
                {
                    tx1 = i;
                    ty1 = i;
                    x0 = tx1 + height;
                    y0 = ty1 - height;
                    x1 = tx1 - height;
                    y1 = ty1 + height;
                    Gfx(x1, y1) += ColorFromPalette(palette, color1, Audio.energy8Scaled);
                    Gfx.drawLine(x0, y0, x1, y1, ColorFromPalette(palette, color2));
                    x0 = tx1 + (height / 2);
                    y0 = ty1 - (height / 2);
                    x1 = tx1 - (height / 2);
                    y1 = ty1 + (height / 2);
                    Gfx(x1, y1) += ColorFromPalette(palette, color3, Audio.energy8Scaled);
                    Gfx.drawLine(x0, y0, x1, y1, ColorFromPalette(palette, color4, Audio.energy8Scaled));
                }
            }
        }
        else
        {
            for (int i = 0; i < Gfx.width(); i++)
            {
                height = 4 * Audio.heights8[i] / Gfx.width();
                if (height > 0)
                {
                    tx1 = Gfx.width() - i;
                    ty1 = i;
                    x0 = tx1 - height;
                    y0 = ty1 - height;
                    x1 = tx1 + height;
                    y1 = ty1 + height;
                    Gfx.drawLine(x0, y0, x1, y1, ColorFromPalette(palette, 16));
                }
            }
        }

        Gfx.randomKaleidoscope(kaleidoscopeMode);
        if (k1or2)
            Gfx.kaleidoscope1();
        else
            Gfx.kaleidoscope2();
        Gfx.spiralStream(31, 31, 64, 128);
    }
};
