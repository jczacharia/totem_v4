#pragma once

class MandalaPattern final : public Pattern
{
    int16_t dx = 0;
    int16_t dy = 0;
    int16_t dz = 0;
    int16_t dsx = 0;
    int16_t dsy = 0;

public:
    static constexpr auto ID = "Mandala";

    MandalaPattern() : Pattern(ID)
    {
    }

    void start(PatternContext& ctx) override
    {
        ctx.noise.randomize();
        dx = random8();
        dy = random16(2000) - 1000;
        dz = random8();
        dsx = random8();
        dsy = random8();
    }

    void render(PatternContext& ctx) override
    {
        if (ctx.audio.isBeat)
        {
            dy = random16(2000) - 1000;
            dx = random16(500) - 250;
            dz = random16(500) - 250;
            ctx.noise.noise_scale_x = random16(10000) + 2000;
            ctx.noise.noise_scale_y = random16(10000) + 2000;
        }

        ctx.noise.noise_y += dy;
        ctx.noise.noise_x += dx;
        ctx.noise.noise_z += dz;

        ctx.noise.fill();

        for (uint8_t i = 0; i < MATRIX_WIDTH; i++)
        {
            for (uint8_t j = 0; j < MATRIX_HEIGHT; j++)
            {
                const uint8_t color = ctx.noise(i, j);
                ctx.leds(i, j) = ColorFromPalette(ctx.targetPalette, color, 150);
            }
        }

        ctx.kaleidoscope3();
        ctx.kaleidoscope1();
    }
};
