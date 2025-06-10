#pragma once

class AudioMandelbrotPattern final : public Pattern
{
    // Mandelbrot parameters (using fixed-point arithmetic scaled by 256)
    int16_t centerX = -128; // -0.5 in fixed point - Mandelbrot center
    int16_t centerY = 0;    // 0.0 in fixed point
    int16_t zoom = 180;     // Zoom level for good Mandelbrot view
    uint8_t maxIterations = 20;

    // Rotation parameters
    uint8_t rotationAngle = 0;
    uint8_t rotationSpeed = 1;

    // Visual parameters
    uint8_t colorOffset = 0;
    uint8_t colorSpeed = 2;
    bool kal12 = false;

  public:
    static constexpr auto ID = "Mandelbrot";

    explicit AudioMandelbrotPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Initialize parameters
        centerX = -128; // -0.5 in fixed point - center on main Mandelbrot body
        centerY = 0;
        zoom = 180; // Good zoom level to see Mandelbrot details
        maxIterations = 20;

        rotationAngle = 0;
        rotationSpeed = 1;
        colorSpeed = 2;

        kal12 = random(0, 2);
        kaleidoscope = random8(0, 2) == 0;
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                kal12 = random(0, 2);
                kaleidoscope = random8(0, 2) == 0;
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
            }
        }

        // Clear the display
        Gfx.clear();

        // Update rotation and color
        rotationAngle += rotationSpeed;
        colorOffset += colorSpeed;
        zoom = 130 - (Audio.energy8Peaks >> 1);

        // Generate fractal
        for (uint8_t py = 0; py < MATRIX_HEIGHT; py++)
        {
            for (uint8_t px = 0; px < MATRIX_WIDTH; px++)
            {
                // Map pixel coordinates to fractal space with rotation
                int16_t dx = px - MATRIX_CENTER_X;
                int16_t dy = py - MATRIX_CENTER_Y;

                // Rotate coordinates using sin8/cos8
                int16_t cos_angle = cos8(rotationAngle) - 128; // -128 to 127
                int16_t sin_angle = sin8(rotationAngle) - 128; // -128 to 127

                // Rotated coordinates (scaled by 128)
                int16_t rotated_x = (dx * cos_angle - dy * sin_angle) >> 7;
                int16_t rotated_y = (dx * sin_angle + dy * cos_angle) >> 7;

                // Map to fractal space
                int16_t x0 = (rotated_x * 512 / zoom) + centerX;
                int16_t y0 = (rotated_y * 512 / zoom) + centerY;

                // Mandelbrot iteration using integer math
                int16_t x = 0;
                int16_t y = 0;
                uint8_t iteration = 0;

                while (iteration < maxIterations)
                {
                    // Calculate x^2 + y^2 (scaled by 256^2)
                    int32_t x2 = ((int32_t)x * x) >> 8;
                    int32_t y2 = ((int32_t)y * y) >> 8;

                    // Check for escape condition (magnitude > 2.0)
                    if (x2 + y2 > 1024) // 4.0 in fixed point
                        break;

                    // Calculate new values: x' = x^2 - y^2 + x0, y' = 2xy + y0
                    int16_t newX = (x2 - y2) + x0;
                    int16_t newY = (((int32_t)x * y) >> 7) + y0; // 2xy

                    x = newX;
                    y = newY;
                    iteration++;
                }

                // Color based on iteration count
                if (iteration < maxIterations)
                {
                    uint8_t hue = colorOffset + iteration * 12;
                    uint8_t brightness = 180 + (iteration << 2);

                    CRGB color = ColorFromPalette(palette, hue, brightness);
                    Gfx(px, py) = color;
                }
            }
        }

        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
            if (kal12)
            {
                Gfx.kaleidoscope2();
            }
            else
            {
                Gfx.kaleidoscope1();
            }
        }

        Gfx.dim(200);
    }
};
