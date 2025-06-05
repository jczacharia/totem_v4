#pragma once

class NoopPattern final : public Pattern
{
  public:
    static constexpr auto ID = "Noop";

    explicit NoopPattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &audio)
        : Pattern(ID, leds, noise, audio)
    {
    }

    void start() override
    {
    }

    void render() override
    {
    }
};
