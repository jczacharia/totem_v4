#pragma once

class AudioClassicSpectrum128Pattern final : public Pattern
{
    bool hueCycle = false;
    byte hue = 0;
    uint32_t hue_ms = 0;
    CRGBPalette16 paletteHeights = randomPalette();
    CRGBPalette16 palettePeaks = randomPalette();

  public:
    static constexpr auto ID = "Classic Spectrum 128";

    explicit AudioClassicSpectrum128Pattern()
        : Pattern(ID)
    {
        hue_ms = millis();
        hueCycle = random8(0, 2);
    }

    void start() override
    {
        paletteHeights = randomPalette();
        do
        {
            palettePeaks = randomPalette();
        } while (paletteHeights == palettePeaks);
    };

    void render() override
    {
        for (byte i = 0; i < MATRIX_WIDTH; i++)
        {
            Gfx.drawLine(i, 0, i, Audio.peaks8[(MATRIX_WIDTH - 1) - i] >> 3, ColorFromPalette(palettePeaks, i * 4));
            Gfx.drawLine(
                i,
                MATRIX_WIDTH - 1,
                i,
                (MATRIX_HEIGHT - 1) - (Audio.peaks8[i] >> 3),
                ColorFromPalette(palettePeaks, i * 4));

            Gfx.drawLine(i, 0, i, Audio.heights8[(MATRIX_WIDTH - 1) - i] >> 3, ColorFromPalette(paletteHeights, i * 4));
            Gfx.drawLine(
                i,
                MATRIX_WIDTH - 1,
                i,
                (MATRIX_HEIGHT - 1) - (Audio.heights8[i] >> 3),
                ColorFromPalette(paletteHeights, i * 4));
        }

        /*
              for (byte i = 0; i < MATRIX_WIDTH; i++)
              {
                //bass
                Gfx.drawLine(i, MATRIX_WIDTH - 1, i, MATRIX_HEIGHT -(Audio.peaks8[i] / 8), i*4, 255,
           LINEARBLEND);
              }

              for (byte i = 0; i < MATRIX_WIDTH; i++)
              {
                //treble
                Gfx.drawLine(i, 0, i, Audio.peaks8Peak[127-i] / 6, i*4, 63, NOBLEND);
                Gfx.drawLine(i, 0, i, Audio.peaks8[127-i] / 6, i*4, 255, LINEARBLEND);
              }
        */

        // cylce through hue, this should be bpm related?
        if (hueCycle && hue_ms + 50 < millis())
        {
            hue++;
            hue_ms = millis();
        }
    }
};
