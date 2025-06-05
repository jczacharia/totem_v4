#pragma once

class AuroraDropPattern final : public Pattern
{
    uint32_t dx = 0, dy = 0, dz = 0;
    CRGBPalette16 palette = randomPalette();
    uint8_t kaleidoscopeEffect = 0;

  public:
    static constexpr auto ID = "AuroraDrop";

    explicit AuroraDropPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        noise.randomize();
        dy = random16(2000) - 1000;
        dx = random16(500) - 250;
        dz = random16(500) - 250;
        noise.noiseScaleX = random16(10000) + 2000;
        noise.noiseScaleY = random16(10000) + 2000;
        palette = randomPalette();
        kaleidoscopeEffect = random8(1, MatrixLeds::KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        noise.noiseX += dx;
        noise.noiseY += dy;
        noise.noiseZ += dz;
        noise.fill();

        for (uint8_t i = 0; i < MATRIX_WIDTH; i++)
        {
            for (uint8_t j = 0; j < MATRIX_HEIGHT; j++)
            {
                const uint8_t color = noise(i, j);
                bkgLeds(i, j) = ColorFromPalette(palette, color, audio.energy8Scaled);
            }
        }

        blur2d(bkgLeds.data(), MATRIX_WIDTH, MATRIX_HEIGHT, 255 - audio.energy8);
        bkgLeds.randomKaleidoscope(kaleidoscopeEffect);
        bkgLeds.kaleidoscope1();
        bkgLeds.dim(50);
    }
};
