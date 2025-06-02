#pragma once

class SpectrumCirclePattern final : public Pattern
{
private:
    uint8_t color1 = 0;
    uint8_t color2 = 64;
    uint8_t color3 = 128;
    uint8_t color4 = 192;

    uint8_t canvasWidth = 0;
    uint8_t canvasCentreX = 0;
    uint8_t canvasCentreY = 0;

    uint8_t colorEffect = 0; // use palette or special (fire, ice, etc.)
    bool cycleColors = false; // todo
    bool insideOut = false; // todo
    bool spin = true; // rotate yes/no
    int spinVal = 0; // value to rotate by
    int spin_factor = 0; // value to rotate by
    bool spinDir = false; // value to rotate by
    bool kaleidoscope = false; // drawn on quarter scrren and apply kaleidoscope effect
    uint8_t kaleidoscopeEffect = 0; // which kaleidoscope effect to apply


public:
    static constexpr auto ID = "SpectrumCircle";

    SpectrumCirclePattern(): Pattern(ID)
    {
    }

    // #------------- START -------------#
    void start(PatternContext& ctx) override
    {
        // determine the colour effect to use. pallete or special (fire, ice, etc.)

        // randomise stuff
        cycleColors = random(0, 2);
        insideOut = random(0, 2);
        spinDir = random(0, 2);
        spin_factor = random(12, 22);
        spin = true;
        if (spinVal == 0) spinVal = random(0, 360);
        // start at random angle for the first time      ### USED TO BREAK STUFF? ###
        kaleidoscope = true;
        kaleidoscopeEffect = random(0, 2);

        // override for testing
        cycleColors = false; // does nothing just yet
        //kaleidoscope = true;
        //kaleidoscopeEffect = 1;


        // half size is commonly chosen, but sometimes others look good/weird
        // consider oscilating these once started
        float scale = 2;
        switch (const uint8_t rnd = random(0, 12))
        {
        case 0:
            scale = 1.25;
            break;
        case 1:
            scale = 1.5;
            break;
        case 2:
            scale = 1.75;
            break;
        case 3:
            scale = 2;
            break;
        case 4:
            scale = 2.5;
            break;
        case 5:
            scale = 3;
            break;
        }
        canvasWidth = MATRIX_WIDTH / scale;
        canvasCentreX = MATRIX_CENTER_X / scale;
        canvasCentreY = MATRIX_CENTER_Y / scale;
        //canvasWidth = canvasWidth / 2;                  // this was a weird bug but causes good effect!!!   now ocassionally scaling canvas randomly
        //canvasCentreX = canvasCentreX / 2;              // ditto
        //canvasCentreY = canvasCentreY / 2;              // ditto
    };


    // #------------- DRAW FRAME -------------#
    void render(PatternContext& ctx) override
    {
        if (ctx.audio.isBeat)
        {
            start(ctx);
        }

        // _order indicates the order in which the effect is drawn, use the appropriate pre and post effects, if any

        static constexpr uint8_t BINS = MATRIX_WIDTH;
        byte audioData = 0;

        // good pair of effects for non caleioscope mode
        //ctx.SpiralStream(31, 31, 64, 128);        // for 64 pixel wide matrix!
        //ctx.DimAll(220);


        constexpr float ratio = 360.0 / BINS; // rotate around 360 degrees
        float rotation = 0.0;
        float angle = 0;

        // add spin to rotation value (0-360)
        if (spinDir)
        {
            spinVal += ctx.audio.energy64f / spin_factor;
        }
        else
        {
            spinVal -= ctx.audio.energy64f / spin_factor;
        }

        spinVal %= 360;

        const uint8_t brightness = ctx.audio.isBeat ? 255 : ctx.audio.energy8Scaled;

        for (int i = 0; i < BINS; i++)
        {
            audioData = ctx.audio.heights8[i] / 6;
            if (constexpr uint8_t maxData = 127; audioData > maxData) audioData = maxData;
            // scale data if in quarter screen
            if (kaleidoscope) audioData = audioData / 2;
            // the radius of the circle is the data value
            rotation = i * ratio;
            rotation = rotation + spinVal;
            if (rotation > 360.0) rotation = rotation - 360.0;
            // calculate angle, then xy
            angle = rotation * 3.14 / 180;
            const int x = static_cast<int>(canvasCentreX + audioData * cos(angle));
            const int y = static_cast<int>(canvasCentreY + audioData * sin(angle));
            if (audioData > 0)
                ctx.drawLine(canvasCentreX, canvasCentreY, x, y,
                             ColorFromPalette(ctx.currentPalette, i * 3, brightness));
            // brighter dots at end of lines this can look bad on quarter screen
            if (ctx.audio.isBeat) ctx.leds(x, y) = CRGB::White;
        }

        switch (kaleidoscopeEffect)
        {
        case 0:
            ctx.kaleidoscope3();
            ctx.spiralStream(31, 31, 64, brightness);
            ctx.kaleidoscope2();
            break;
        case 1:
            ctx.kaleidoscope3();
            ctx.kaleidoscope2();
            ctx.spiralStream(31, 31, 64, brightness);
            break;
        default:
            break;
        }
    }
};
