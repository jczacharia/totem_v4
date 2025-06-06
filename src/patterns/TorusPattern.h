#pragma once

class TorusPattern final : public Pattern
{
    // used for internal colour cycling
    uint8_t color1 = 0;
    uint8_t color2 = 64;
    uint8_t color3 = 128;
    uint8_t color4 = 192;
    // used for internal audio calcs
    byte audioData = 0;
    uint8_t maxData = 127;

    // pattern specific parameters used for randomizing, theses are mostly radomized at initial start and
    // determine the style of backdrop and main effects to render
    bool cycleColors = true; // cylce through the color spectrum or not
    bool dimAll = false;
    uint8_t dimAmount = 0;

    void randomize()
    {
        cycleColors = random8(0, 3);  // 75% of the time, the color palette will be cycled
        kaleidoscope = random8(0, 2); // 50% chance of kaleidoscope
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        if (kaleidoscopeMode == 5)
            kaleidoscopeMode = 3;
        dimAll = random8(0, 10); // 90% chance of dimming the output render
        dimAmount = random8(200, 240);
    }

  public:
    static constexpr auto ID = "Torus";

    TorusPattern()
        : Pattern(ID)
    {
    }

    // #------------- START -------------#
    void start() override
    {
        // randomize the effects to use
        randomize();
        palette = randomPalette();
    };

    uint8_t theta1 = 0;
    uint8_t theta2 = 0;
    uint8_t minx = 16;
    uint8_t miny = 16;
    uint8_t maxx = 48;
    uint8_t maxy = 48;
    uint8_t spirooffset = 16; // 32= hexagon,  16=
    uint8_t radiusx = 8;
    uint8_t radiusy = 12;
    uint8_t hueoffset = 0;

    uint8_t lastx = 0;
    uint8_t lasty = 0;

    // #------------- DRAW FRAME -------------#
    void render() override
    {
        // order - indicates the order in which the effect is being drawn, patterns can use the appropriate pre and post
        // effects, if any, depending on order etc. total - the total number of audio effect animations being played
        // simultaineously

        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        // this patterns looks good with dimming so do it sometimes
        if (dimAll)
        {
            Gfx.dim(dimAmount);
        }

        // general cyclic stuff
        if (cycleColors)
        {
            // currently cycling through colours as fast as possible
            color1++;
            color2++;
        }

        // scale audio data to floor at specDataMinVolume

        for (int i = 0; i < 17; i++)
        {
            // determine brightness from

            const uint8_t x1 = mapsin8(theta1 + i * spirooffset, minx, maxx);
            const uint8_t y1 = mapcos8(theta1 + i * spirooffset, miny, maxy);
            const uint8_t x2 = mapsin8(theta2 + i * spirooffset, x1 - radiusx, x1 + radiusx);
            const uint8_t y2 = mapcos8(theta2 + i * spirooffset, y1 - radiusy, y1 + radiusy);

            const uint8_t audio = Audio.heights8[(BINS - 1) * i / 16];
            const CRGB color = ColorFromPalette(palette, hueoffset + i * spirooffset, audio);

            if (i > 0)
            {
                GfxCanvasH.drawLine(lastx / 2, lasty / 2, x2 / 2, y2 / 2, color);
            }

            Gfx.applyOther(GfxCanvasH, (x1 / 2) - 8, (y1 / 2) - 8, 1.5);

            lastx = x2;
            lasty = y2;
        }

        theta1 = theta1 + 2;
        theta2++;
        hueoffset++;

        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
};
