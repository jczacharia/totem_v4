#pragma once

class AuroraDropPattern final : public Pattern
{
    uint32_t x = 0, y = 0, v_time = 0, hue_time = 0, hxy = 0;
    uint8_t octaves = 1;
    uint8_t hue_octaves = 3;
    int xscale = 57771;
    int yscale = 57771;
    int hue_scale = 1;
    int time_speed = 1111;
    int hue_speed = 31;
    int x_speed = 331;
    int y_speed = 1111;

  public:
    static constexpr auto ID = "Aurora Drop";

    AuroraDropPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        hxy = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
        x = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
        y = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
        v_time = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
        hue_time = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
        Noise.randomize();
        kaleidoscopeMode = random8(1, Gfx.KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        fill_2dnoise16(
            GfxCanvasQ.data(),
            GfxCanvasQ.width(),
            GfxCanvasQ.height(),
            false,
            octaves,
            x,
            xscale,
            y,
            yscale,
            v_time,
            hue_octaves,
            hxy,
            hue_scale,
            hxy,
            hue_scale,
            hue_time,
            false);

        size_t bin = 0;
        for (size_t x = 0; x < GfxCanvasQ.width(); x++)
        {
            for (size_t y = 0; y < GfxCanvasQ.height(); y++)
            {
                GfxCanvasQ(x, y).nscale8(std::min(255.0f, 1.25f * Audio.heights8[bin++]));
                if (bin >= Audio.heights8.size())
                {
                    bin = 0;
                }
            }
        }

        Gfx.applyOther(GfxCanvasQ, 0, 0);
        Gfx.applyOther(GfxCanvasQ, Gfx.centerX() / 2, 0);
        Gfx.applyOther(GfxCanvasQ, 0, Gfx.centerY() / 2);
        Gfx.applyOther(GfxCanvasQ, Gfx.centerX() / 2, Gfx.centerY() / 2);
        blur2d(Gfx.data(), Gfx.width(), Gfx.height(), Audio.energy8 / 2);

        GfxBkg.randomKaleidoscope(kaleidoscopeMode);
        Gfx.kaleidoscope1();

        x += x_speed;
        y += y_speed;
        v_time += time_speed;
        hue_time += hue_speed;
    }
};
