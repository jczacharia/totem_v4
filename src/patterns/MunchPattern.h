#pragma once

class MunchPattern final : public Pattern
{
    byte count = 0;
    byte dir = 1;
    byte flip = 0;
    byte generation = 0;

  public:
    static constexpr auto ID = "Munch";

    MunchPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        count = 0;
        dir = 1;
        flip = 0;
        generation = 0;
        palette = randomPalette();
    };

    void render() override
    {

        if (count >= GfxBkg.width() || count < 0)
        {
            dir = dir * -1;
        }

        for (int x = 0; x < GfxBkg.width(); x++)
        {
            for (int y = 0; y < GfxBkg.height(); y++)
            {
                GfxBkg(x + 0, y + 0) += (x + 0 ^ y + 0 ^ flip) < count
                                            ? ColorFromPalette(palette, ((x + 0 ^ y + 0) << 3) + generation)
                                            : CRGB::Black;
                GfxBkg(x + 1, y + 0) += (x + 1 ^ y + 0 ^ flip) < count
                                            ? ColorFromPalette(palette, ((x + 1 ^ y + 0) << 3) + generation)
                                            : CRGB::Black;
                GfxBkg(x + 0, y + 1) += (x + 0 ^ y + 1 ^ flip) < count
                                            ? ColorFromPalette(palette, ((x + 0 ^ y + 1) << 3) + generation)
                                            : CRGB::Black;
                GfxBkg(x + 1, y + 1) += (x + 1 ^ y + 1 ^ flip) < count
                                            ? ColorFromPalette(palette, ((x + 1 ^ y + 1) << 3) + generation)
                                            : CRGB::Black;
            }
        }

        count += dir;

        if (count <= 0)
        {
            if (flip == 0)
                flip = 7; // 31
            else
                flip = 0;
        }

        generation++;
    }

    void backgroundPostProcess() override
    {
        GfxBkg.dim(25);
    }
};
