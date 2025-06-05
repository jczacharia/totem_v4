#pragma once

#include "Pattern.h"

class TestPattern final : public Pattern
{
    uint8_t x_ = 0;
    uint32_t ms_ = 0;

  public:
    static constexpr auto ID = "Test";

    explicit TestPattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &audio)
        : Pattern(ID, leds, noise, audio)
    {
    }

    void start() override
    {
    }

    void render() override
    {
        if (millis() - ms_ > 5000)
        {
            ms_ = millis();
            x_++;
            Serial.printf("x: %d\n", x_);
        }

        switch (x_)
        {
            case 0: spiralStream(31, 31, 64, 255); break;
            case 1: spiralStreamVer2(31, 31, 64, 255); break;
            case 2: expand(31, 31, 64, 255); break;
            case 3: streamRight(33, 31, 31, 64, 255); break;
            case 4: streamLeft(33, 31, 31, 64, 255); break;
            case 5: streamDown(10); break;
            case 6: streamUp(10); break;
            case 7: streamUpAndLeft(10); break;
            case 8: streamUpAndRight(10); break;
            case 9: moveDown(); break;
            case 10: moveX(10); break;
            case 11: moveY(10); break;
            case 12: verticalMoveFrom(10, 30); break;
            case 13: copy(10, 20, 30, 40, 50, 60); break;
            case 14: rotateTriangle(); break;
            case 15: mirrorTriangle(); break;
            default: break;
        }
    }
};
