#pragma once

class MandalaPattern final : public Pattern
{
    int16_t dx = 0;
    int16_t dy = 0;
    int16_t dz = 0;
    uint8_t kaleidoscopeEffect = 0;
    CRGBPalette16 palette = randomPalette();

  public:
    static constexpr auto ID = "Mandala";

    explicit MandalaPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        noise.randomize();
        dy = random16(2000) - 1000;
        dx = random16(500) - 250;
        dz = random16(500) - 250;
        palette = randomPalette();
        kaleidoscopeEffect = random8(1, MatrixLeds::KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        if (audio.isBeat)
        {
            if (audio.totalBeats % 4 == 0)
            {
                dy = random16(2000) - 1000;
                dx = random16(500) - 250;
                dz = random16(500) - 250;
                noise.noiseScaleX = random16(10000) + 2000;
                noise.noiseScaleY = random16(10000) + 2000;
            }
        }

        noise.noiseY += dy;
        noise.noiseX += dx;
        noise.noiseZ += dz;

        noise.fill();

        for (uint8_t i = 0; i < MATRIX_WIDTH; i++)
        {
            for (uint8_t j = 0; j < MATRIX_HEIGHT; j++)
            {
                const uint8_t color = noise(i, j);
                bkgLeds(i, j) = ColorFromPalette(palette, color, 255);
            }
        }

        bkgLeds.randomKaleidoscope(kaleidoscopeEffect);
        bkgLeds.kaleidoscope1();
        bkgLeds.dim(35);
    }
};
