#pragma once

class CirclesPattern final : public Pattern
{
    static constexpr size_t CIRCLES_COUNT = 6;
    uint8_t mode = 0;
    uint8_t hue = 255;

  public:
    static constexpr auto ID = "Circles";

    CirclesPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        kaleidoscope = random(0, 3);
        mode = random(0, 5);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                kaleidoscope = random(0, 3);
                mode = random(0, 5);
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
            }
        }

        if (mode == 0)
        {
            for (int i = 0; i < BINS; ++i)
            {
                const uint8_t audio = (Audio.energy8 + Audio.heights8[i]) / 2;
                const CRGB color = ColorFromPalette(palette, 255 * i / (BINS - 1), audio);
                Gfx.drawCircle(Gfx.centerX(), Gfx.centerY(), Gfx.centerX() * audio / 255, color);
            }

            blur2d(Gfx.data(), Gfx.width(), Gfx.height(), Audio.energy8 / 2);

            if (kaleidoscope)
            {
                Gfx.randomKaleidoscope(kaleidoscopeMode);
            }
        }
        else if (mode == 1)
        {
            for (int i = 0; i < BINS; ++i)
            {
                const uint8_t audio = (Audio.energy8 + Audio.heights8[i]) / 2;
                const CRGB color = ColorFromPalette(palette, 255 * i / (BINS - 1), audio);
                Gfx.drawCircle(Gfx.centerX() / 2, Gfx.centerY() / 2, (Gfx.centerX() / 2) * audio / 255, color);
            }

            blur2d(Gfx.data(), Gfx.width(), Gfx.height(), Audio.energy8 / 2);

            if (kaleidoscope)
            {
                Gfx.randomKaleidoscope(kaleidoscopeMode);
            }
            Gfx.kaleidoscope2();
        }
        else
        {
            uint8_t prevRange = 0;
            const uint8_t circles = map(Audio.energy8, 0, 255, 6, 42);
            for (int i = 0; i < circles; i++)
            {
                const uint8_t range = (BINS - 1) * (i + 1) / circles;
                const uint8_t height = std::min(255.0f, 1.5f * Audio.avgHeights8Range(prevRange, range));
                const CRGB color = ColorFromPalette(palette, height, height);
                prevRange = range;
                Gfx.drawCircle(Gfx.centerX(), Gfx.centerY(), Gfx.centerX() * i / (circles - 1), color);
            }

            blur2d(Gfx.data(), Gfx.width(), Gfx.height(), Audio.energy8 / 1.5);

            if (kaleidoscope)
            {
                Gfx.randomKaleidoscope(kaleidoscopeMode);
            }
        }
    }
};
