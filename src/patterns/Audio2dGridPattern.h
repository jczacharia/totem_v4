#pragma once

class Audio2dGridPattern : public Pattern
{
    bool hueCycle = true;
    byte hue = 0;
    uint32_t hue_ms = 0;

    uint8_t mode = false;
    bool drawHorizontal = false;
    bool drawDiagonal = false;
    bool drawDoubleVerticalLines = false;
    bool drawDoubleHorizontalLines = false;
    bool drawDoubleDiagonalLines1 = false;
    bool drawDoubleDiagonalLines2 = false;
    CRGBPalette16 palette = randomPalette();

    void randomize()
    {
        drawDoubleVerticalLines = random8(0, 2);
        drawDoubleHorizontalLines = random8(0, 2);
        drawDoubleDiagonalLines1 = random8(0, 2);
        drawDoubleDiagonalLines2 = random8(0, 2);
        mode = random8(0, 3);
        palette = randomPalette();
    }

  public:
    static constexpr auto ID = "2D Grid";

    Audio2dGridPattern()
        : Pattern(ID)
    {
    }

    // ------------------ start -------------------
    void start() override
    {

        hue_ms = millis();
        hueCycle = random8(0, 5);
        hue = random8(0, 255);
        randomize();
    }

    // --------------------- draw frame -------------------------
    void render() override
    {
        int x, y;
        byte aud;

        if (mode == 0)
        {
            x = 0;
            y = 10;
            for (int i = 0; i < 120; i++)
            {

                aud = Audio.heights8[(MATRIX_WIDTH - 1) * i / 119] / 32;
                Gfx.drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2)));
                if (drawDoubleVerticalLines)
                    Gfx.drawLine(x + 1, y, x + 1, y - aud, ColorFromPalette(palette, hue + (i * 2)));

                x = x + 4;
                if (x > MATRIX_WIDTH - 1)
                {
                    x = 0;
                    y = y + 9;
                }
            }
        }
        else if (mode == 1)
        {
            x = 1;
            y = 0;
            for (int i = 0; i < 120; i++)
            {

                aud = Audio.heights8[(MATRIX_WIDTH - 1) * i / 119] / 32;
                Gfx.drawLine(x, y, x + aud, y, ColorFromPalette(palette, hue + (i * 2)));
                if (drawDoubleHorizontalLines)
                    Gfx.drawLine(x, y + 1, x + aud, y + 1, ColorFromPalette(palette, hue + (i * 2)));

                y = y + 4;
                if (y > MATRIX_HEIGHT - 1)
                {
                    y = 0;
                    x = x + 9;
                }
            }
        }
        else
        {
            x = 0;
            y = 10;
            for (int i = 0; i < 120; i++)
            {
                aud = Audio.heights8[(MATRIX_WIDTH - 1) * i / 119] / 32;
                Gfx.drawLine(x, y, x + aud, y - aud, ColorFromPalette(palette, hue + (i * 2)));
                if (drawDoubleDiagonalLines1)
                    Gfx.drawLine(x, y + 1, x + aud, y - aud + 1, ColorFromPalette(palette, hue + (i * 2)));
                if (drawDoubleDiagonalLines2)
                    Gfx.drawLine(x + 1, y + 1, x + aud + 1, y - aud + 1, ColorFromPalette(palette, hue + (i * 2)));

                x = x + 4;
                if (x > MATRIX_WIDTH - 1)
                {
                    x = 0;
                    y = y + 9;
                }
            }
        }

        if (hueCycle && hue_ms + 50 < millis())
        {
            hue++;
            hue_ms = millis();
        }
    }
};
