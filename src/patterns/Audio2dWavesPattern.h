#pragma once

class Audio2dWavesPattern final : public Pattern
{
    bool hueCycle = true;
    byte hue = 0;
    uint32_t hue_ms = 0;

    bool drawVertical = false;
    bool drawHorizontal = false;
    bool drawDiagonal = false;
    bool drawDoubleVerticalLines = false;
    bool drawDoubleHorizontalLines = false;
    bool drawDoubleDiagonalLines1 = false;
    bool drawDoubleDiagonalLines2 = false;
    int direction = 1;

    bool effect1 = false;
    CRGBPalette16 palette = randomPalette();

  public:
    static constexpr auto ID = "2D Waves";

    Audio2dWavesPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {

        hue_ms = millis();

        // randomise
        hueCycle = random8(0, 5); // mostly cycle through the the hues
        hue = random8(0, 255);

        effect1 = random8(0, 2);

        drawDoubleVerticalLines = random8(0, 2);
        drawDoubleHorizontalLines = random8(0, 2);
        drawDoubleDiagonalLines1 = random8(0, 2);
        drawDoubleDiagonalLines2 = random8(0, 2);
        drawVertical = random8(0, 2);
        drawHorizontal = random8(0, 2);
        drawDiagonal = random8(0, 2);
        if (!drawVertical && !drawHorizontal && !drawDiagonal)
            drawVertical = true;

        // drawVertical = false;
        // drawHorizontal = false;
        // drawDiagonal = true;

        palette = randomPalette();
    }

    void render() override
    {

        int x, y;
        int oldx = 0, oldy = 0;
        int xStart = 12;
        int yStart = 17;
        int aud;
        int oldaud = 0;

        Gfx.dim(150);

        x = 0;
        y = 8;
        for (int i = 0; i < 128; i++)
        {
            if (x != 0)
            {
                aud = direction * (Audio.heights8[(MATRIX_WIDTH - 1) * i / 127] / 16);
                Gfx.drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2)));
                oldx = x;
                oldy = y - aud;
            }
            else
            {
                oldx = x;
                oldy = y;
            }

            if (aud != oldaud || !effect1)
                direction = -direction;
            oldaud = aud;

            x = x + 2;
            if (x > MATRIX_WIDTH - 1)
            {
                x = 0;
                y = y + 16;
            }
        }

        // cylce through hue, this should be bpm related?
        if (hueCycle && hue_ms + 50 < millis())
        {
            hue++;
            hue_ms = millis();
        }
    }
};
