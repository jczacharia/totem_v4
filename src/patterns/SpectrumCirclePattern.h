#pragma once

class SpectrumCirclePattern final : public Pattern
{
    uint8_t color1 = 0;
    uint8_t color2 = 64;
    uint8_t color3 = 128;
    uint8_t color4 = 192;
    uint8_t canvasWidth = 0;
    uint8_t canvasCentreX = 0;
    uint8_t canvasCentreY = 0;
    uint8_t colorEffect = 0;
    bool cycleColors = false;
    bool insideOut = false;
    bool spin = true;
    int spinVal = 0;
    int spin_factor = 0;
    bool spinDir = false;
    bool kaleidoscope = false;
    uint8_t kaleidoscopeEffect = 0;
    CRGBPalette16 palette = randomPalette();

  public:
    static constexpr auto ID = "SpectrumCircle";

    SpectrumCirclePattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &audio)
        : Pattern(ID, leds, noise, audio)
    {
    }

    void start() override
    {

        cycleColors = random(0, 2);
        insideOut = random(0, 2);
        spinDir = random(0, 2);
        spin_factor = random(12, 22);
        spin = true;
        if (spinVal == 0)
        {
            spinVal = random(0, 360);
        }

        kaleidoscope = true;
        kaleidoscopeEffect = random(0, 2);

        cycleColors = false;

        float scale = 2;
        switch (random(0, 6))
        {
            case 0: scale = 1.25; break;
            case 1: scale = 1.5; break;
            case 2: scale = 1.75; break;
            case 3: scale = 2; break;
            case 4: scale = 2.5; break;
            case 5: scale = 3; break;
            default: break;
        }
        canvasWidth = MATRIX_WIDTH / scale;
        canvasCentreX = MATRIX_CENTER_X / scale;
        canvasCentreY = MATRIX_CENTER_Y / scale;
    };

    void render() override
    {
        if (audio.isBeat)
        {
            if (audio.totalBeats % 4 == 0)
            {
                start();
            }
        }

        static constexpr uint8_t BINS = MATRIX_WIDTH;
        byte audioData = 0;

        constexpr float ratio = 360.0 / BINS;
        float rotation = 0.0;
        float angle = 0;

        if (spinDir)
        {
            spinVal += audio.energy64f / spin_factor;
        }
        else
        {
            spinVal -= audio.energy64f / spin_factor;
        }

        spinVal %= 360;

        const uint8_t brightness = audio.isBeat ? 255 : audio.energy8Scaled;

        for (int i = 0; i < BINS; i++)
        {
            audioData = audio.heights8[i] / 6;
            if (constexpr uint8_t maxData = 127; audioData > maxData)
            {
                audioData = maxData;
            }

            if (kaleidoscope)
            {
                audioData = audioData / 2;
            }

            rotation = i * ratio;
            rotation = rotation + spinVal;
            if (rotation > 360.0)
            {
                rotation = rotation - 360.0;
            }

            angle = rotation * 3.14 / 180;
            const int x = static_cast<int>(canvasCentreX + audioData * cos(angle));
            const int y = static_cast<int>(canvasCentreY + audioData * sin(angle));
            drawLine(canvasCentreX, canvasCentreY, x, y, ColorFromPalette(palette, i * 3, brightness));

            if (audio.isBeat)
            {
                leds(x, y) = CRGB::White;
            }
        }

        switch (kaleidoscopeEffect)
        {
            case 0:
                kaleidoscope3();
                spiralStream(31, 31, 64, brightness);
                if (audio.totalBeats % 3 == 0)
                {
                    kaleidoscope2();
                }
                else
                {
                    kaleidoscope1();
                }
                break;
            case 1:
                kaleidoscope3();
                if (audio.totalBeats % 3 == 0)
                {
                    kaleidoscope2();
                }
                else
                {
                    kaleidoscope1();
                }
                spiralStream(31, 31, 64, brightness);
                break;
            default: break;
        }
    }
};
