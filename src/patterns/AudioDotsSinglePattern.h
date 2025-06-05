#pragma once

class AudioDotsSinglePattern : public Pattern
{
    uint8_t dimVal;
    uint8_t dimStart;

    uint8_t x0, y0, x1, y1;
    bool vertical = false;
    bool diagonalLines = true;
    bool diagonalOffset = 4;

    // randomise theses
    uint8_t dimEnd;
    uint8_t colorSpread = 1;
    bool upwards = true;
    uint8_t audioScale;
    bool caleido = 0;
    uint8_t caleidoEffect;
    bool hueCycle = true;
    byte hue = 0;
    uint32_t hue_ms;

    uint8_t rolling = 0;

    long rollDelay = 100;

    long ms_previous;

    CRGBPalette16 palette = randomPalette();

  public:
    static constexpr auto ID = "AudioDotsSingle";

    explicit AudioDotsSinglePattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &audio)
        : Pattern(ID, leds, noise, audio)
    {
    }

    // ------------------ START ------------------
    void start() override
    {

        hue_ms = millis();
        dimStart = 255;
        dimVal = dimStart;

        // randomise stuff
        dimEnd = random8(128, 218);
        caleido = random(0, 3);        // 1 in 3 chance of not getting caleidoscope
        caleidoEffect = random8(1, 3); // only chose effect 1 or 2
        vertical = random(0, 2);
        upwards = random(0, 2);
        colorSpread = random(2, 5);
        audioScale = random(6, 11); // 6 and above is good
        hueCycle = random8(0, 5);   // mostly cycle through the the hues
        hue = random8(0, 255);

        ms_previous = millis();
        palette = randomPalette();

        // caleido = true;
        // vertical = true;
        // upwards = true;
        // hueCycle = true;
    };

    // --------------- DRAW FRAME ----------------
    void render() override
    {

        // if we are going to dim all, then do it gradually
        leds.dim(dimVal);
        if (dimVal > dimEnd)
            dimVal--;
        // effects.DimAll(128);

        for (byte i = 0; i < MATRIX_WIDTH; i++)
        {
            uint8_t data1 = audio.heights8[i] / audioScale;
            if (data1 > MATRIX_CENTER_Y - 1)
                data1 = MATRIX_CENTER_Y - 1;

            x0 = i;
            y0 = data1;

            // roll the dots sideways
            x0 = x0 + rolling;
            if (x0 > MATRIX_WIDTH)
                x0 = x0 - MATRIX_WIDTH;

            const CRGB color1 = ColorFromPalette(palette, (i * colorSpread) + hue);

            if (vertical)
            {
                if (upwards)
                {
                    drawPixel(y0, x0, color1);
                    drawPixel(MATRIX_WIDTH - y0, x0, color1);
                }
                else
                {
                    drawPixel(y0, MATRIX_HEIGHT - x0, color1);
                    drawPixel(MATRIX_WIDTH - y0, MATRIX_HEIGHT - x0, color1);
                }
            }
            else
            {
                if (upwards)
                {
                    drawPixel(x0, MATRIX_CENTER_Y - y0, color1);
                    drawPixel(x0, MATRIX_CENTER_Y + y0, color1);
                }
                else
                {
                    drawPixel(x0, y0, color1);
                    drawPixel(x0, MATRIX_HEIGHT - y0, color1);
                }
            }
        }

        if (caleido)
        {
            randomKaleidoscope(caleidoEffect);
        }

        // roll the display sideways one more pixel
        if (ms_previous + rollDelay < millis())
        {
            rolling++;
            if (rolling >= MATRIX_WIDTH)
                rolling = 0;
            ms_previous = millis();
        }

        // cylce through hue, this should be bpm related?
        if (hueCycle && hue_ms + 50 < millis())
        {
            hue++;
            hue_ms = millis();
        }
    }
};
