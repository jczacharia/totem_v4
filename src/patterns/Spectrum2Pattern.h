#pragma once

class Spectrum2Pattern final : public Pattern
{
    uint8_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    bool mirror = false;
    uint8_t mirrorDim = 0;
    uint8_t dimAmount = 0;
    bool useCurrentPalette = false;
    uint8_t colorSpread = 0;
    uint8_t kaleidoscopeEffect = 0;

public:
    static constexpr auto ID = "Spectrum2";

    Spectrum2Pattern() : Pattern(ID)
    {
    }

    // #############
    // ### START ###
    // #############
    void start(PatternContext& ctx) override
    {
        mirror = true;
        mirrorDim = 255;
        dimAmount = random(0, 10);
        useCurrentPalette = true;
        colorSpread = 7;
        kaleidoscopeEffect = random(0, 2);
    };

    // ##################
    // ### DRAW FRAME ###
    // ##################
    void render(PatternContext& ctx) override
    {
        if (ctx.audio.isBeat)
        {
            start(ctx);
        }

        // always apply small dim
        ctx.leds.dim(250 - (dimAmount * 10));

        // centered spectrum analyzer
        for (uint i = 0; i < MATRIX_WIDTH / 2; i++)
        {
            const uint8_t data = ctx.audio.heights8[2 * i] >> 2;
            x1 = i;
            x2 = i;
            y1 = MATRIX_HEIGHT - 1;
            y2 = MATRIX_HEIGHT - data;

            const uint8_t space = 64 - (y1 - y2);
            y1 = y1 - (space / 2);
            y2 = y2 - (space / 2);

            if (data)
            {
                if (useCurrentPalette)
                {
                    ctx.drawLine(x1, y1, x2, y2,
                                 ColorFromPalette(ctx.currentPalette, i * colorSpread, ctx.audio.energy8Scaled));
                    if (mirror)
                    {
                        ctx.drawLine(MATRIX_WIDTH - x1 - 1, y1, MATRIX_WIDTH - x2 - 1, y2,
                                     ColorFromPalette(ctx.currentPalette, i * colorSpread, mirrorDim));
                    }
                }
                else
                {
                    ctx.drawLine(x1, y1, x2, y2,
                                 ColorFromPalette(ctx.currentPalette, 16, ctx.audio.energy8Scaled));
                    if (mirror)
                    {
                        ctx.drawLine(MATRIX_WIDTH - x1 - 1, y1, MATRIX_WIDTH - x2 - 1, y2,
                                     ColorFromPalette(ctx.currentPalette, 16, mirrorDim));
                    }
                }
            }
        }

        switch (kaleidoscopeEffect)
        {
        case 0:
            ctx.kaleidoscope1();
            break;
        default:
            break;
        }
    }
};
