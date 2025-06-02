#pragma once

class AudioTrianglesPattern final : public Pattern
{
    bool cycleColors = false;
    uint8_t color1 = 0;
    uint8_t color2 = 64;
    uint8_t color3 = 128;
    uint8_t color4 = 192;
    uint8_t kaleidoscopeMode = 0;

public:
    static constexpr auto ID = "AudioTriangles";

    AudioTrianglesPattern(): Pattern(ID)
    {
    }

    void start(PatternContext& ctx) override
    {
        cycleColors = true;
        kaleidoscopeMode = random8(0, 3);
    };

    void render(PatternContext& ctx) override
    {
        if (ctx.audio.isBeat)
        {
            start(ctx);
        }

        if (cycleColors)
        {
            color1++;
            color2++;
            color3++;
            color4++;
        }
        else
        {
            color1 = 0;
            color2 = 64;
            color3 = 128;
            color4 = 192;
        }

        uint8_t x1 = ctx.audio.heights8[10] >> 3;
        uint8_t y1 = ctx.audio.heights8[20] >> 3;
        uint8_t x2 = ctx.audio.heights8[30] >> 3;
        uint8_t y2 = ctx.audio.heights8[40] >> 3;
        uint8_t x3 = ctx.audio.heights8[50] >> 3;
        uint8_t y3 = ctx.audio.heights8[60] >> 3;

        ctx.drawLine(x1, y1, x2, y2, ColorFromPalette(ctx.currentPalette, color1, 200));
        ctx.drawLine(x2, y2, x3, y3, ColorFromPalette(ctx.currentPalette, color2, 200)); // green
        ctx.drawLine(x3, y3, x1, y1, ColorFromPalette(ctx.currentPalette, color3, 200)); // green

        x1 = ctx.audio.heights8[13] >> 3;
        y1 = ctx.audio.heights8[23] >> 3;
        x2 = ctx.audio.heights8[33] >> 3;
        y2 = ctx.audio.heights8[43] >> 3;
        x3 = ctx.audio.heights8[53] >> 3;
        y3 = ctx.audio.heights8[63] >> 3;

        ctx.drawLine(x1, y1, x2, y2, ColorFromPalette(ctx.currentPalette, color2, 200));
        ctx.drawLine(x2, y2, x3, y3, ColorFromPalette(ctx.currentPalette, color3, 200)); // green
        ctx.drawLine(x3, y3, x1, y1, ColorFromPalette(ctx.currentPalette, color4, 200)); // green

        switch (kaleidoscopeMode)
        {
        case 0:
            ctx.kaleidoscope3();
            ctx.kaleidoscope1();
            break;
        case 1:
            ctx.kaleidoscope4();
            ctx.kaleidoscope1();
            break;
        case 2:
            ctx.kaleidoscope1();
            break;
        default:
            ctx.kaleidoscope2();
            break;
        }
    }
};
