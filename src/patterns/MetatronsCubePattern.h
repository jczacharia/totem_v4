#pragma once

#include <vector>

class MetatronsCubePattern final : public Pattern
{
    // Circle parameters for Metatron's Cube
    struct Circle
    {
        float x;
        float y;
        float radius;
    };

    std::vector<Circle> circles;
    uint8_t hueShift = 0;
    uint8_t brightnessWave = 0;
    uint8_t lineHue = 0;

    // Draw a circle outline
    void drawCircle(float cx, float cy, float radius, CRGB color)
    {
        const int steps = 40; // Number of points to draw the circle
        for (int i = 0; i < steps; i++)
        {
            const float angle1 = (2.0f * PI * i) / steps;
            const float angle2 = (2.0f * PI * (i + 1)) / steps;

            const int x1 = cx + radius * cos(angle1);
            const int y1 = cy + radius * sin(angle1);
            const int x2 = cx + radius * cos(angle2);
            const int y2 = cy + radius * sin(angle2);

            Gfx.drawLine(x1, y1, x2, y2, color);
        }
    }

    // Draw all connecting lines between circle centers
    void drawConnectingLines(uint8_t brightness)
    {
        // Connect every circle center to every other circle center
        for (size_t i = 0; i < circles.size(); i++)
        {
            for (size_t j = i + 1; j < circles.size(); j++)
            {
                const Circle &c1 = circles[i];
                const Circle &c2 = circles[j];

                // Calculate distance-based color
                const float dist = sqrt((c1.x - c2.x) * (c1.x - c2.x) + (c1.y - c2.y) * (c1.y - c2.y));
                const uint8_t colorIndex = static_cast<uint8_t>(lineHue + dist * 2) % 255;

                // Vary brightness based on distance - closer connections are brighter
                const uint8_t lineBrightness = scale8(brightness, map(dist, 0, MATRIX_WIDTH, 255, 100));
                const CRGB lineColor = ColorFromPalette(palette, colorIndex, lineBrightness);

                Gfx.drawLine(c1.x, c1.y, c2.x, c2.y, lineColor);
            }
        }
    }

  public:
    static constexpr auto ID = "Metatron's Cube";

    MetatronsCubePattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        circles.clear();

        // Calculate radius based on matrix size
        const float baseRadius = min(MATRIX_WIDTH, MATRIX_HEIGHT) / 9.0f;
        const float centerX = MATRIX_CENTER_X;
        const float centerY = MATRIX_CENTER_Y;

        // Metatron's Cube has 13 circles total

        // 1. Center circle
        circles.push_back({centerX, centerY, baseRadius});

        // 2. Inner ring - 6 circles (Flower of Life pattern)
        const float innerDistance = baseRadius * 2.0f;
        for (int i = 0; i < 6; i++)
        {
            const float angle = (2.0f * PI * i) / 6.0f;
            const float x = centerX + innerDistance * cos(angle);
            const float y = centerY + innerDistance * sin(angle);
            circles.push_back({x, y, baseRadius});
        }

        // 3. Outer ring - 6 more circles (vertices of hexagon)
        const float outerDistance = baseRadius * 3.464f; // 2 * sqrt(3) * radius
        for (int i = 0; i < 6; i++)
        {
            const float angle = (2.0f * PI * i) / 6.0f + (PI / 6.0f); // 30 degree offset
            const float x = centerX + outerDistance * cos(angle);
            const float y = centerY + outerDistance * sin(angle);
            circles.push_back({x, y, baseRadius});
        }

        hueShift = random8();
        lineHue = random8();
        palette = randomPalette();
    }

    void render() override
    {
        // Update color animations
        hueShift += 1;                         // Slow hue rotation for circles
        lineHue += 2;                          // Slightly faster rotation for lines
        brightnessWave = beatsin8(4, 80, 255); // Slow breathing effect

        // Draw connecting lines first (so circles appear on top)
        const uint8_t lineBrightness = scale8(brightnessWave, 150);
        drawConnectingLines(lineBrightness);

        // Draw filled circles with glowing effect
        for (size_t idx = 0; idx < circles.size(); idx++)
        {
            const Circle &circle = circles[idx];

            // Fill circle with gradient effect
            for (int dx = -circle.radius; dx <= circle.radius; dx++)
            {
                for (int dy = -circle.radius; dy <= circle.radius; dy++)
                {
                    const float dist = sqrt(dx * dx + dy * dy);
                    if (dist <= circle.radius)
                    {
                        const int px = circle.x + dx;
                        const int py = circle.y + dy;

                        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT)
                        {
                            // Create gradient from center to edge
                            const uint8_t distRatio = map(dist, 0, circle.radius, 0, 255);
                            const uint8_t colorIndex = (hueShift + idx * 20) % 255;

                            // Center is brighter, edges fade out
                            uint8_t brightness = scale8(255 - distRatio, brightnessWave);

                            // Special brightness for center circle
                            if (idx == 0)
                            {
                                brightness = qadd8(brightness, 50);
                            }

                            const CRGB color = ColorFromPalette(palette, colorIndex, brightness);
                            Gfx(px, py) += color;
                        }
                    }
                }
            }

            // Draw circle outline for definition
            const uint8_t outlineColorIndex = (hueShift + idx * 20 + 128) % 255;
            const uint8_t outlineBrightness = scale8(brightnessWave, 200);
            const CRGB outlineColor = ColorFromPalette(palette, outlineColorIndex, outlineBrightness);
            drawCircle(circle.x, circle.y, circle.radius, outlineColor);
        }

        // Add subtle noise sparkles
        if (Audio.totalBeats % 2 == 0)
        {
            for (int i = 0; i < 10; i++)
            {
                const uint8_t x = random8(MATRIX_WIDTH);
                const uint8_t y = random8(MATRIX_HEIGHT);
                const uint8_t sparkle = inoise8(x * 20, y * 20, millis() / 10) > 230 ? 100 : 0;
                if (sparkle > 0)
                {
                    Gfx(x, y) += CRGB(sparkle, sparkle, sparkle);
                }
            }
        }
    }
};
