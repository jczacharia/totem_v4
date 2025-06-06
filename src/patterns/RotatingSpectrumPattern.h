#pragma once

#include "Geometry.h"

class RotatingSpectrumPattern final : public Pattern
{
  private:
    uint8_t dimVal;
    uint8_t dimStart;

    // randomise theses
    uint8_t dimEnd;
    bool hueCycle1 = false;
    byte hue1 = 0;
    uint32_t hue_ms1;
    uint8_t offset1 = 0;
    uint8_t counter1 = 0;
    uint8_t mirror1 = 0;
    uint8_t stagger1 = 1;

    CRGBPalette16 palette = randomPalette();
    uint8_t colorSpread;
    uint8_t vertical;
    bool backdrop;
    bool kaleidoscope;
    uint8_t kaleidoscopeMode;
    uint8_t audioScale;

    bool hueCycle2 = false;
    byte hue2 = 0;
    uint32_t hue_ms2;
    uint8_t offset2 = 0;
    uint8_t counter2 = 0;
    uint8_t mirror2 = 0;
    uint8_t stagger2 = 1;

    int x0, y0, x1, y1, x2, y2, x3, y3;
    int diagonalOffset = 0;

    Point p1, p2;
    Point rp1, rp2;
    float cx = 0;
    float cy = 0;
    float angle = 0;

  public:
    static constexpr auto ID = "Rotating Spectrum";

    explicit RotatingSpectrumPattern()
        : Pattern(ID)
    {
    }

    uint8_t zzzz = 0;

    // ------------------ start -------------------
    void start() override
    {
        dimStart = 255;
        dimVal = dimStart;

        // randomise stuff
        dimEnd = random8(128, 255);

        hue1 = random8(0, 255);
        hue_ms1 = millis();
        hueCycle1 = true; // random8(0,2);
        offset1 = 0;
        counter1 = 0;
        mirror1 = random8(0, 4);
        stagger1 = random8(0, 5); // if (stagger==3) stagger = 4;

        hue2 = random8(0, 255);
        hue_ms2 = millis();
        hueCycle2 = true; // random8(0,2);
        offset2 = 0;
        counter2 = 0;
        mirror2 = random8(0, 4);
        stagger2 = random8(0, 5); // if (stagger==3) stagger = 4;

        palette = randomPalette();
        colorSpread = random(1, 5);
        vertical = random8(0, 3);
        backdrop = random8(0, 2);
        kaleidoscope = random8(0, 5); // 80% chance of kaleidoscope
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        audioScale = random8(5, 9);
        // useCurrentPalette = true;
        // colorSpread = 4;
        // kaleidoscope = true;

        Noise.randomize();

        p1.x = 9;
        p1.y = 31;
        p2.x = 53;
        p2.y = 31;
    }

    // --------------------- DRAW FRAME -------------------------
    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                colorSpread = random(1, 5);
                vertical = random8(0, 3);
                backdrop = random8(0, 2);
                kaleidoscope = random8(0, 5);
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
                audioScale = random8(5, 7);
            }
        }

        // if we are going to dim all, then do it gradually
        Gfx.dim(dimVal);
        if (dimVal > dimEnd)
            dimVal--;
        // leds.dim(180);

        //         // draw to half width canvas
        //         Matrix::ClearCanvas(1);

        // draw on full screen

        /*
              for (byte i = 0; i < 64; i=i+4)
              {
                data = fftData.specData16[i/4] / 3;    // use the 16 bins for this!
                if (data > 63) data = 63;
                x1 = i;
                x2 = i+4;
                y1 = data;
                y2 = y1 + diagonalOffset;
                // only draw bars if there is non zero data
                if (data)
                  drawLine(x1, y1, x2, y2, dma_display->color565(128, 128, 128));
              }
        */

        // centered spectrum analyser

        /*
            for (byte i = 0; i < MATRIX_WIDTH / 2; i++)
            {
              data = fftData.specData32[i] / 3;
              x1 = i + (MATRIX_WIDTH/4);
              x2 = i + (MATRIX_WIDTH/4);
              y1 = MATRIX_HEIGHT - 1;
              y2 = MATRIX_HEIGHT - data;

              space = 64 - (y1 - y2);
              y1 = y1 - (space/2);
              y2 = y2 - (space/2);

              Point p1(x1,y1);
              Point p2(x2,y2);
              //float angle2 = 1;

              Point rp1 = rotate_point(31,31,angle,p1);
              Point rp2 = rotate_point(31,31,angle,p2);

              if (data)
              {
                if (useCurrentPalette)
                {
                  drawLine(rp1.x, rp1.y, rp2.x, rp2.y, i * colorSpread, 255);
                  drawLineCanvas(Matrix::canvasH, rp1.x-16, rp1.y-16, rp2.x-16, rp2.y-16, i * colorSpread,
           128);
                }
                else
                {
                  drawLine(rp1.x, rp1.y, rp2.x, rp2.y, 16, 255);
                  drawLineCanvas(Matrix::canvasH, rp1.x-16, rp1.y-16, rp2.x-16, rp2.y-16, 16, 128);
                }
              }

            }
        */

        /*
            for (byte i = 0; i < MATRIX_WIDTH; i++)
            {
              data = fftData.specData[i+32] / 3;
              x1 = i;
              x2 = i;
              y1 = MATRIX_HEIGHT - 1;
              y2 = MATRIX_HEIGHT - data;

              space = 64 - (y1 - y2);
              y1 = y1 - (space/2);
              y2 = y2 - (space/2);

              Point p1(x1,y1);
              Point p2(x2,y2);
              //float angle2 = 1;

              Point rp1 = rotate_point(31,31,angle,p1);
              Point rp2 = rotate_point(31,31,angle,p2);

              if (data)
              {
                if (useCurrentPalette)
                {
                  drawLine(rp1.x, rp1.y, rp2.x, rp2.y, i * colorSpread, 255);
                  drawLineCanvas(Matrix::canvasH, rp1.x - 16, rp1.y - 16, rp2.x - 16, rp2.y - 16, i *
           colorSpread, 128);
                }
            else
                {
                  drawLine(rp1.x, rp1.y, rp2.x, rp2.y, 16, 255);
                  drawLineCanvas(Matrix::canvasH, rp1.x - 16, rp1.y - 16, rp2.x - 16, rp2.y - 16, 16, 128);
                }
              }

            }
        */
        for (byte i = 0; i < 96; i++)
        {
            const uint8_t p_i = i * 63 / 95;
            const uint8_t data = Audio.heights8[p_i] / audioScale;
            x1 = i - 16;
            x2 = i - 16;
            y1 = MATRIX_HEIGHT - 1;
            y2 = MATRIX_HEIGHT - data;

            const uint8_t space = 64 - (y1 - y2);
            y1 = y1 - (space / 2);
            y2 = y2 - (space / 2);

            Point rp1(x1, y1);
            Point rp2(x2, y2);
            // float angle2 = 1;

            rp1.rotate(31, 31, angle);
            rp2.rotate(31, 31, angle);

            if (data)
            {
                Gfx.drawLine(
                    rp1.x, rp1.y, rp2.x, rp2.y, ColorFromPalette(palette, i * colorSpread, Audio.energy8Peaks));
                // Matrix::BresLineCanvasH(Matrix::canvasH.data(), rp1.x - 16, rp1.y - 16, rp2.x - 16,
                // rp2.y - 16,
                //                         (i * colorSpread) + 128, Audio.energy8);
                if (vertical)
                {
                    Gfx.drawLine(rp1.y, rp1.x, rp2.y, rp2.x, ColorFromPalette(palette, i * colorSpread, Audio.energy8));
                    // Matrix::BresLineCanvasH(Matrix::canvasH.data(), rp1.y - 16, rp1.x - 16, rp2.y - 16,
                    // rp2.x - 16,
                    //                         (i * colorSpread) + 128, Audio.energy8);
                }
            }
        }

        // Matrix::ApplyCanvas(Matrix::canvasH, 16, 0, 1);
        // Matrix::ApplyCanvasMirror(Matrix::canvasH, 16, 16, 1);

        //         //Matrix::ApplyCanvas(Matrix::canvasH, 31, 0, 0.5);
        //         //Matrix::ApplyCanvas(Matrix::canvasH, 0, 0, 2.0);
        //         if (backdrop)
        //         {
        //             Matrix::ApplyCanvasHMirror(Matrix::canvasH.data(), 0, 0, 2.0);
        //         }

        angle = angle + 0.01;
        if (angle >= 3.14 * 2)
            angle = 0;

        if (hueCycle1 && hue_ms1 + 200 < millis())
        {
            hue_ms1 = millis();
            hue1++;
        }

        if (hueCycle2 && hue_ms2 + 200 < millis())
        {
            hue_ms2 = millis();
            hue2++;
        }

        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
            if (Audio.totalBeats % 3 == 0)
            {
                Gfx.kaleidoscope2();
            }
            else
            {
                Gfx.kaleidoscope1();
            }
        }
    }
};
