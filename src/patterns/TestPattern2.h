#pragma once

#include "Pattern.h"

class TestPattern2 final : public Pattern
{
    uint8_t e = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t r = 0;
    uint32_t ms = 0;

public:
    static constexpr auto ID = "Test2";

    explicit TestPattern2() : Pattern(ID)
    {
    }

    void start(PatternContext& ctx) override
    {
    }

    void render(PatternContext& ctx) override
    {
        if (millis() - ms > 5000)
        {
            ms = millis();
            x++;
            Serial.printf("x: %d\n", x);
        }

        if (ctx.audio.totalBeats % 4 == 0)
        {
            x = random8(0, MATRIX_WIDTH);
            y = random8(0, MATRIX_HEIGHT);
            r = random8(0, MATRIX_CENTER_X);
        }

        for (int8_t i = MATRIX_CENTER_X - 1; i >= 0; --i)
        {
            ctx.fillCircle<CRGB>(MATRIX_CENTER_X,MATRIX_CENTER_Y, i,
                                 ColorFromPalette(RainbowColors_p, 255 * i / (MATRIX_CENTER_X - 1), 255));
        }

        // switch (e)
        // {
        // case 0:
            ctx.spiralStream(x, y, r, 255);
        //     break;
        // default:
        //     break;
        // }
    }
};
