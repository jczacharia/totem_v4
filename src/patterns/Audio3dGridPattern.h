#pragma once

class Audio3dGridPattern final : public Pattern
{
    bool hueCycle = true;
    byte hue = 0;
    uint32_t hue_ms = 0;
    bool drawVertical = false;
    bool drawWave = true;
    CRGBPalette16 palette = randomPalette();

    void randomize()
    {
        drawVertical = random8(0, 2);
        drawWave = random8(0, 2);
        if (!drawVertical && !drawWave)
            drawVertical = true;

        palette = randomPalette();
    }

  public:
    static constexpr auto ID = "Audio3dGrid";

    explicit Audio3dGridPattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &aud)
        : Pattern(ID, leds, noise, aud)
    {
    }

    void start() override
    {

        hue_ms = millis();

        // randomise
        hueCycle = random8(0, 5); // mostly cycle through the the hues
        hue = random8(0, 255);

        randomize();
    }

    void render() override
    {
        int x, y;
        int oldx = 0, oldy = 0;
        int xStart = 12;
        int yStart = 17;
        byte aud;

        leds.dim(150);

        for (int i = 0; i < 121; i++)
        {
            aud = audio.heights8[(MATRIX_WIDTH - 1) * i / 120] / 20;

            if (drawVertical)
            {
                if (i < 11)
                {
                    x = xStart + (i * 5);
                    y = yStart + (i * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 11 && i < 22)
                {
                    x = xStart - 1 + ((i - 11) * 5);
                    y = yStart + 2 + ((i - 11) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 22 && i < 33)
                {
                    x = xStart - 2 + ((i - 22) * 5);
                    y = yStart + 4 + ((i - 22) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 33 && i < 44)
                {
                    x = xStart - 3 + ((i - 33) * 5);
                    y = yStart + 6 + ((i - 33) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 44 && i < 55)
                {
                    x = xStart - 4 + ((i - 44) * 5);
                    y = yStart + 8 + ((i - 44) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 55 && i < 66)
                {
                    x = xStart - 5 + ((i - 55) * 5);
                    y = yStart + 10 + ((i - 55) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 66 && i < 77)
                {
                    x = xStart - 6 + ((i - 66) * 5);
                    y = yStart + 12 + ((i - 66) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 77 && i < 88)
                {
                    x = xStart - 7 + ((i - 77) * 5);
                    y = yStart + 14 + ((i - 77) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 88 && i < 99)
                {
                    x = xStart - 8 + ((i - 88) * 5);
                    y = yStart + 16 + ((i - 88) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 99 && i < 110)
                {
                    x = xStart - 9 + ((i - 99) * 5);
                    y = yStart + 18 + ((i - 99) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }

                if (i >= 110 && i < 121)
                {
                    x = xStart - 10 + ((i - 110) * 5);
                    y = yStart + 20 + ((i - 110) * 2);
                    drawLine(x, y, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                    drawLine(x, y - aud, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 255));
                }
            }

            if (drawWave)
            {
                // draw wave like pattern
                if (i < 11)
                {
                    x = xStart + (i * 5);
                    y = yStart + (i * 2);
                    if (i != 0)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 11 && i < 22)
                {
                    x = xStart - 1 + ((i - 11) * 5);
                    y = yStart + 2 + ((i - 11) * 2);
                    if (i != 11)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 22 && i < 33)
                {
                    x = xStart - 2 + ((i - 22) * 5);
                    y = yStart + 4 + ((i - 22) * 2);
                    if (i != 22)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 33 && i < 44)
                {
                    x = xStart - 3 + ((i - 33) * 5);
                    y = yStart + 6 + ((i - 33) * 2);
                    if (i != 33)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 44 && i < 55)
                {
                    x = xStart - 4 + ((i - 44) * 5);
                    y = yStart + 8 + ((i - 44) * 2);
                    if (i != 44)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 55 && i < 66)
                {
                    x = xStart - 5 + ((i - 55) * 5);
                    y = yStart + 10 + ((i - 55) * 2);
                    if (i != 55)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 66 && i < 77)
                {
                    x = xStart - 6 + ((i - 66) * 5);
                    y = yStart + 12 + ((i - 66) * 2);
                    if (i != 66)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 77 && i < 88)
                {
                    x = xStart - 7 + ((i - 77) * 5);
                    y = yStart + 14 + ((i - 77) * 2);
                    if (i != 77)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 88 && i < 99)
                {
                    x = xStart - 8 + ((i - 88) * 5);
                    y = yStart + 16 + ((i - 88) * 2);
                    if (i != 88)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 99 && i < 110)
                {
                    x = xStart - 9 + ((i - 99) * 5);
                    y = yStart + 18 + ((i - 99) * 2);
                    if (i != 99)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }

                if (i >= 110 && i < 121)
                {
                    x = xStart - 10 + ((i - 110) * 5);
                    y = yStart + 20 + ((i - 110) * 2);
                    if (i != 110)
                        drawLine(oldx, oldy, x, y - aud, ColorFromPalette(palette, hue + (i * 2), 128));
                }
            }

            oldx = x;
            oldy = y - aud;
        }

        // cylce through hue, this should be bpm related?
        if (hueCycle && hue_ms + 50 < millis())
        {
            hue++;
            hue_ms = millis();
        }
    }
};
