#pragma once

class AudioClassicSpectrum128Pattern : public Pattern
{
    bool hueCycle = false;
    byte hue = 0;
    uint32_t hue_ms = 0;
    CRGBPalette16 paletteHeights = randomPalette();
    CRGBPalette16 palettePeaks = randomPalette();

  public:
    static constexpr auto ID = "AudioClassicSpectrum128";

    explicit AudioClassicSpectrum128Pattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &audio)
        : Pattern(ID, leds, noise, audio)
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
            drawLine(i, 0, i, audio.peaks8[(MATRIX_WIDTH - 1) - i] / 7, ColorFromPalette(palettePeaks, i * 4));
            drawLine(
                i, MATRIX_WIDTH - 1, i, MATRIX_HEIGHT - (audio.peaks8[i] / 7), ColorFromPalette(palettePeaks, i * 4));

            drawLine(i, 0, i, audio.heights8[(MATRIX_WIDTH - 1) - i] / 9, ColorFromPalette(paletteHeights, i * 4));
            drawLine(
                i,
                MATRIX_WIDTH - 1,
                i,
                MATRIX_HEIGHT - (audio.heights8[i] / 9),
                ColorFromPalette(paletteHeights, i * 4));
        }

        // linearBlend = false;

        /*
              for (byte i = 0; i < MATRIX_WIDTH; i++)
              {
                //bass
                drawLine(i, MATRIX_WIDTH - 1, i, MATRIX_HEIGHT -(audio.peaks8[i] / 8), i*4, 255,
           LINEARBLEND);
              }

              for (byte i = 0; i < MATRIX_WIDTH; i++)
              {
                //treble
                drawLine(i, 0, i, audio.peaks8Peak[127-i] / 6, i*4, 63, NOBLEND);
                drawLine(i, 0, i, audio.peaks8[127-i] / 6, i*4, 255, LINEARBLEND);
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
