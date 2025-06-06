#pragma once
#include <Microphone.h>

class BigSparkPattern final : public Pattern
{
    // not using these at the moment
    uint8_t color1 = 0;
    uint8_t color2 = 64;
    uint8_t color3 = 128;
    uint8_t color4 = 192;

    uint8_t canvasWidth = 0;
    uint8_t canvasCentreX = 0;
    uint8_t canvasCentreY = 0;

    // theses are mostly radomised at initial start
    uint8_t paletteType = 0;  // TODO: use standard random palette or special (fire, ice, etc.)
    uint8_t colorEffect = 0;  // TODO:
    bool cycleColors = false; // todo: cylce through the color spectrum
    bool insideOut = false; // todo: instead of starting lines at centre and go out, start at fixed radiusand draw line
                            // inwards towards the centre
    int8_t spin = 1;        // 1/-1=rotate clockwise/counter, 0=no rotate
    int spinVal = 0;        // value to rotate by
    bool backdrop = true;   // draw a scaled backdrop
    uint8_t sprites = 0;    // will we used half width sprites, if so how many....
    uint8_t spriteBehaviour = 0; // 0=stationary, 1=revolving around screen
    bool dimPreRender = false;   // dim prior to frame render
    uint8_t dimAmount = 0;

  public:
    static constexpr auto ID = "Big Spark";

    BigSparkPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {

        // randomly determine the colour effect to use, etc.
        cycleColors = random(0, 2); // TODO:
        insideOut = random(0, 2);   // TODO:
        spin = random(0, 2);
        if (spin == 0)
            spin = -1; // 1=clockwise, -1 = anticlockwise, 0=no spin (disabled))
        if (spinVal == 0)
            spinVal = random(0, 360); // choose random angle at initial start      ### USED TO BREAK STUFF? ###
        kaleidoscope = random(0, 2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        dimPreRender = random8(0, 2);
        dimAmount = random8(150, 250);

        // override for testing
        cycleColors = false; // does nothing just yet
        // kaleidoscope = true;
        // kaleidoscopeMode = 0;
        backdrop = true;
        dimAmount = 150;

        // setup parameters depending on random stuff
        if (kaleidoscope)
        {
            // half size is commonly chosen, but sometimes others look good/weird
            // consider oscilating these once started
            float scale = 2;
            uint8_t rnd = random(0, 12); // half the time, use the standard, other half randomizes some scales
            switch (rnd)
            {
                case 0: scale = 1.25; break;
                case 1: scale = 1.5; break;
                case 2: scale = 1.75; break;
                case 3: scale = 2; break;
                case 4: scale = 2.5; break;
                case 5: scale = 3; break;
            }
            canvasWidth = Gfx.width() / scale;
            canvasCentreX = Gfx.centerX() / scale;
            canvasCentreY = Gfx.centerY() / scale;
            // canvasWidth = canvasWidth / 2;                  // this was a weird bug but causes good effect!!!   now
            // ocassionally scaling canvas randomly canvasCentreX = canvasCentreX / 2;              // ditto
            // canvasCentreY = canvasCentreY / 2;              // ditto
        }
        else
        {
            canvasWidth = Gfx.width();
            canvasCentreX = Gfx.centerX();
            canvasCentreY = Gfx.centerY();
        }
    };

    // #------------- DRAW FRAME -------------#
    void render() override
    {
        // _order indicates the order in which the effect is drawn, use the appropriate pre and post effects, if any

        // static const uint8_t BINS = BINS;
        byte audioData;
        uint8_t maxData = 127;

        // good pair of effects for non caleioscope mode
        // effects.SpiralStream(31, 31, 64, 128);        // for 64 pixel wide matrix!

        // dimming is useful on this pattern and can clean things up
        if (dimPreRender)
        {
            Gfx.dim(dimAmount);
        }

        float radius = canvasWidth / 2;
        float ratio = 360.0 / BINS; // rotate around 360 degrees
        float rotation = 0.0;
        float angle = 0;
        int x, y;

        // add spin to rotation value (0-360)
        spinVal += spin;
        if (spinVal > 360)
            spinVal = 0;
        if (spinVal < 0)
            spinVal = 360;

        // --------- draw any canvas requirements --------------- #

        // determine if we want to first draw to a blank half width canvas first, or directly to the existing screen
        if (backdrop || sprites > 0)
        {
            uint8_t canvasScale = 2;
            uint8_t audioScale = canvasScale;
            uint8_t centreX = (Gfx.width() / 2) / canvasScale;
            uint8_t centreY = (Gfx.height() / 2) / canvasScale;
            for (int i = 0; i < BINS; i++)
            {
                audioData = Audio.heights8[i] / 12;
                if (audioData > maxData)
                    audioData = maxData;
                audioData = audioData / audioScale;
                // the radius of the circle is the data value
                radius = audioData;
                rotation = i * ratio;
                rotation = rotation + spinVal;
                if (rotation > 360.0)
                    rotation = rotation - 360.0;
                // calculate angle, then xy
                angle = rotation * 3.14 / 180;
                x = (int)(centreX + radius * cos(angle));
                y = (int)(centreY + radius * sin(angle));
                if (audioData > 0)
                    GfxCanvasH.drawLine(centreX, centreY, x, y, ColorFromPalette(palette, i * 2.65, 255));
            }

            // two sprite moving up and down
            Gfx.applyOther(GfxCanvasH, 0, Gfx.height() - (beatSineOsciWidth[3] + 16), 1);
            Gfx.applyOther(GfxCanvasH, 32, beatSineOsciWidth[3] - 16, 1);

            // two orbiting sprites
            Gfx.applyOther(GfxCanvasH, beatSineOsciWidth[3] - 8, beatCosineOsciWidth[3] - 8, 0.5);
            Gfx.applyOther(
                GfxCanvasH, Gfx.width() - (beatSineOsciWidth[3] + 8), Gfx.height() - (beatCosineOsciWidth[3] + 8), 0.5);

            // another two orbiting other way
            Gfx.applyOther(GfxCanvasH, beatSineOsciWidth[3] - 8, Gfx.height() - (beatCosineOsciWidth[3] + 8), 0.5);
            Gfx.applyOther(GfxCanvasH, Gfx.width() - (beatSineOsciWidth[3] + 8), beatCosineOsciWidth[3] - 8, 0.5);

            if (backdrop)
            {
                Gfx.applyOther(GfxCanvasH, -32, -32, 4.0, 128); // 128 = medium blur
            }
        }

        Gfx.randomKaleidoscope(kaleidoscopeMode);
        if (kaleidoscope)
        {
            Gfx.kaleidoscope1();
        }
        else
        {
            Gfx.kaleidoscope2();
        }

        // really good pair of effects
        // effects.SpiralStream(31, 31, 64, 128);        // for 64 pixel wide matrix!
        // effects.DimAll(220);

        // don't dim at end, removes clarity
        // effects.DimAll(220);
    }
};
