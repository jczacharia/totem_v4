﻿#pragma once

class Audio8x8SquaresPattern final : public Pattern
{
  private:
    // used for internal colour cycling
    uint8_t color1 = 0;
    uint8_t color2 = 64;
    uint8_t color3 = 128;
    uint8_t color4 = 192;
    // used for internal scaling functions
    uint8_t canvasWidth;
    uint8_t canvasCentreX;
    uint8_t canvasCentreY;
    // used for internal audio calcs
    byte audioData;
    uint8_t maxData = 127;

    // pattern specific parameters used for randomizing, theses are mostly radomized at initial start and
    // determine the style of backdrop and main effects to render
    // general/common parameters
    bool generalRand1 = false; // general use, either true/false
    bool generalRand2 = false; // general use
    uint8_t colorEffect = 0;   // TODO: use standard palette or special (fire, ice, etc.)
    bool cycleColors = true;   // cylce through the color spectrum or not
    uint8_t audioRange = 0;    // 0=render full audio range, 1=bass centric, 2=mids, 3=treble
    // backdrop only random stuff
    uint8_t backdrop = 0; // 0=don't draw backdrop, 1-4=draw various scaled backdrops (override if there are alreadt too
                          // many backdrops?)
    // main effects random stuff
    uint8_t sprites = 0;         // will we used half width sprites, if so how many....
    uint8_t spriteBehaviour = 0; // 0=stationary, 1=revolving around screen

  public:
    static constexpr auto ID = "8x8 Squares";

    Audio8x8SquaresPattern()
        : Pattern(ID)
    {
    }

    // #------------- START -------------#
    void start() override
    {

        // randomize the effects to use
        generalRand1 = random(0, 2); // for stream effect directions
        cycleColors = random(0, 3);  // 75% of the time, the color palette will be cycled
        backdrop =
            random(1, 5); // 1 in 5 chance of no background being rendered, otherwise averge chance to any of the 4
        audioRange = random8(0, 4); // same chance for getting any of the 5 audio ranges

        // override some randomizations if previous patterns are over active, or for any other reason
        // if (PatternsAudioBackdropCount > 2 && random8(0,5)) backdrop = 0;              // if too many previous
        // backdrops, then only a 1 in 5 chance of rendering another backdrop if (PatternsAudioMainEffectCount > 2 &&
        // rendering another effect

        // ## ----- override random stuff for testing ----- ##
        // backdrop = 3;
        if (backdrop == 0)
            backdrop = 4;
        // ## ----- override random stuff for testing ----- ##

        // generate random canvas, sprites etc. to use in animations

        // setup parameters depending on random stuff
        if (!kaleidoscope)
        {
            canvasWidth = MATRIX_WIDTH;
            canvasCentreX = MATRIX_CENTER_X;
            canvasCentreY = MATRIX_CENTER_Y;
        }
        else
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
            canvasWidth = MATRIX_WIDTH / scale;
            canvasCentreX = MATRIX_CENTER_X / scale;
            canvasCentreY = MATRIX_CENTER_Y / scale;
            // canvasWidth = canvasWidth / 2;                  // this was a weird bug but causes good effect!!!   now
            // ocassionally scaling canvas randomly canvasCentreX = canvasCentreX / 2;              // ditto
            // canvasCentreY = canvasCentreY / 2;              // ditto
        }

        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    };

    // #------------- DRAW FRAME -------------#
    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
                backdrop = random(1, 5);
                generalRand1 = random(0, 2);
                audioRange = random8(0, 4);
            }
        }

        // order - indicates the order in which the effect is being drawn, patterns can use the appropriate pre and post
        // effects, if any, depending on order etc. total - the total number of audio effect animations being played
        // simultaineously

        // todo
        float radius = canvasWidth / 2;
        float ratio = 360.0 / BINS; // rotate around 360 degrees
        float rotation = 0.0;
        float angle = 0;
        int x, y;

        // general cyclic stuff
        if (cycleColors)
        {
            // currently cycling through colours as fast as possible
            color1++;
            color2++;
        }

        // ---------------------- generate canvas/sprites ----------------------------

        // single canvas for use in the backdrop or main animations
        if (backdrop || sprites > 0)
        {
            uint8_t canvasScale = 4;
            uint8_t canvasWidth = MATRIX_WIDTH / canvasScale;
            uint8_t canvasHeight = MATRIX_HEIGHT / canvasScale;
            uint8_t centreX = (MATRIX_WIDTH / 2) / canvasScale;
            uint8_t centreY = (MATRIX_HEIGHT / 2) / canvasScale;
            uint8_t audioScale = canvasScale * 4;

            // draw simple quarter width square to scale up for a background
            for (int i = 0; i < canvasWidth; i++)
            {
                // randomise the audio range to sample
                switch (audioRange)
                {
                    case 0:
                        // full range
                        audioData = Audio.heights8[i] / audioScale;
                        break;
                    case 1:
                        // bass centric
                        audioData = Audio.heights8[i] / audioScale;
                        break;
                    case 2:
                        // mids centric
                        audioData = Audio.heights8[i + 8] / audioScale;
                        break;
                    case 3:
                        // treble centric
                        audioData = Audio.heights8[i + 16] / audioScale;
                        break;
                }
                if (audioData >= canvasHeight)
                    audioData = canvasHeight - 1;

                if (audioData > 0)
                {
                    // BresenhamLineCanvasH(i, canvasHeight - audioData, i, 0,
                    // ColorFromPalette(palette,(i*16) + color1, 32)); BresenhamLineCanvasH(i,
                    // canvasHeight - 1, i, canvasHeight - audioData, ColorFromPalette(palette,(i*16) + color2,
                    // 255));
                    GfxCanvasH.drawLine(
                        i, canvasHeight - audioData, i, 0, ColorFromPalette(palette, (i * 16) + color1, Audio.energy8));
                    GfxCanvasH.drawLine(

                        i, canvasHeight - 1, i, canvasHeight - audioData, ColorFromPalette(palette, (i * 16) + color2));
                }
            }

            // ---------------------- generate chosen backdrop ----------------------------

            if (backdrop == 1)
            {
                for (int i = 0; i < canvasScale; i++)
                {
                    Gfx.applyOther(GfxCanvasH, i * canvasWidth, 0, 1);
                    Gfx.applyOther(GfxCanvasH, i * canvasWidth, 16, 1);
                    Gfx.applyOther(GfxCanvasH, i * canvasWidth, 32, 1);
                    Gfx.applyOther(GfxCanvasH, i * canvasWidth, 48, 1);
                }
                // ApplyCanvas(0, 0, 4.0);
                Gfx.dim(180);
            }

            if (backdrop == 2)
            {
                Gfx.applyOther(GfxCanvasH, 16, 16, 2.0, 128); // 64 = light blur
            }

            // overscaled half width canvas centered on frame/screen
            if (backdrop == 3)
            {
                Gfx.applyOther(GfxCanvasH, 0, 0, 4.0, 64); // 64 = light blur
            }

            if (backdrop == 4)
            {
                Gfx.applyOther(GfxCanvasH, 16, 16, 2.0, 128); // 64 = light blur
                Gfx.applyOther(GfxCanvasH, 0, 0, 4.0, 64);    // 64 = light blur
            }

            // apply test effects
            if (backdrop == 1)
            {
                if (generalRand1)
                {
                    if (generalRand2)
                    {
                        Gfx.streamRight(128);
                    }
                    else
                    {
                        Gfx.streamUpAndRight(128);
                    }
                }
                else
                {
                    Gfx.streamUp(128);
                }
            }
        }

        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
            Gfx.kaleidoscope2();
            Gfx.kaleidoscope1();
        }
    }
};
