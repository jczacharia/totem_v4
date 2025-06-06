#pragma once

class NoopPattern final : public Pattern
{
  public:
    static constexpr auto ID = "Noop";

    explicit NoopPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
    }

    void render() override
    {
    }
};
