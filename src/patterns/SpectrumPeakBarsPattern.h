#pragma once

class SpectrumPeakBarsPattern final : public Pattern
{
    uint8_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    bool flipVert = false;
    uint8_t diagonalLines = 4;
    uint8_t diagonalOffset = 0;

  public:
    static constexpr auto ID = "Spectrum Peak Bars";

    SpectrumPeakBarsPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        flipVert = random(0, 2);     // 50% chance of bveing flipped
        kaleidoscope = random(0, 3); // 1 in 3 change of not getting kaleidoscope
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        diagonalLines = random(0, 10); // 1 in 10 chance we'll get dialgonal lines
        diagonalOffset = diagonalLines ? 0 : 4;
        palette = randomPalette();
    }

    void render() override
    {
        // 16 simple peak only bars
        // no kaleidoscope version
        if (!kaleidoscope)
        {
            // draw on full screen
            for (byte i = 0; i < 64; i = i + 4)
            {
                uint8_t data = Audio.heights8[i] / 4; // use the 16 bins for this!
                x1 = i;
                x2 = i + 4;
                if (flipVert)
                {
                    y1 = data;
                    y2 = y1 + diagonalOffset;
                }
                else
                {
                    y1 = Gfx.height() - data;
                    y2 = y1 - diagonalOffset;
                }
                // only draw bars if there is non zero data
                Gfx.drawLine(x1, y1, x2, y2, ColorFromPalette(palette, 255 * i / 63));
            }
        }
        else
        {
            for (byte i = 0; i < 32; i = i + 2)
            {
                uint8_t data = Audio.heights8[i / 2] / 8;
                if (data > 31)
                    data = 31;
                x1 = i;
                x2 = i + 4;
                if (flipVert)
                {
                    y1 = data;
                    y2 = y1 + (diagonalOffset / 2);
                }
                else
                {
                    y1 = Gfx.height() - data - 32;
                    y2 = y1 - (diagonalOffset / 2);
                }
                // only draw bars if there is non zero data
                Gfx.drawLine(x1, y1, x2, y2, ColorFromPalette(palette, 255 * i / 31));
            }

            Gfx.randomKaleidoscope(kaleidoscopeMode);
            Gfx.kaleidoscope2();
        }
    }
};
