#pragma once

#include "Pattern.h"

class TestPattern final : public Pattern
{
    uint8_t x = 0;
    uint32_t ms = 0;

public:
    static constexpr auto ID = "Test";

    explicit TestPattern() : Pattern(ID)
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

        switch (x)
        {
        case 0:
            ctx.spiralStream(31, 31, 64, 255);
            break;
        case 1:
            ctx.spiralStreamVer2(31, 31, 64, 255);
            break;
        case 2:
            ctx.expand(31, 31, 64, 255);
            break;
        case 3:
            ctx.streamRight(33, 31, 31, 64, 255);
            break;
        case 4:
            ctx.streamLeft(33, 31, 31, 64, 255);
            break;
        case 5:
            ctx.streamDown(10);
            break;
        case 6:
            ctx.streamUp(10);
            break;
        case 7:
            ctx.streamUpAndLeft(10);
            break;
        case 8:
            ctx.streamUpAndRight(10);
            break;
        case 9:
            ctx.moveDown();
            break;
        case 10:
            ctx.moveX(10);
            break;
        case 11:
            ctx.moveY(10);
            break;
        case 12:
            ctx.verticalMoveFrom(10, 30);
            break;
        case 13:
            ctx.copy(10, 20, 30, 40, 50, 60);
            break;
        case 14:
            ctx.rotateTriangle();
            break;
        case 15:
            ctx.mirrorTriangle();
            break;
        default:
            break;
        }
    }
};
