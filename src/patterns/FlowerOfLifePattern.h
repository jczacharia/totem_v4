#pragma once

#include <vector>

class FlowerOfLifePattern final : public Pattern
{
    // Circle parameters for the Flower of Life
    struct Circle
    {
        float x;
        float y;
        float radius;
    };

    std::vector<Circle> circles;
    uint8_t hueShift = 0;
    uint8_t brightnessWave = 0;

    // Draw a circle outline
    void drawCircle(float cx, float cy, float radius, CRGB color)
    {
        // Use parametric circle equation
        const int steps = 60; // Number of points to draw the circle
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

    // Check if a point is inside any circle
    bool isInsideFlower(float px, float py)
    {
        for (const auto &circle : circles)
        {
            const float dx = px - circle.x;
            const float dy = py - circle.y;
            if (dx * dx + dy * dy <= circle.radius * circle.radius)
            {
                return true;
            }
        }
        return false;
    }

    // Count how many circles a point is inside (for petal effect)
    uint8_t getOverlapCount(float px, float py)
    {
        uint8_t count = 0;
        for (const auto &circle : circles)
        {
            const float dx = px - circle.x;
            const float dy = py - circle.y;
            if (dx * dx + dy * dy <= circle.radius * circle.radius)
            {
                count++;
            }
        }
        return count;
    }

  public:
    static constexpr auto ID = "Flower of Life";

    FlowerOfLifePattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        circles.clear();

        // Calculate radius based on matrix size
        const float baseRadius = min(MATRIX_WIDTH, MATRIX_HEIGHT) / 5.0f;
        const float centerX = MATRIX_CENTER_X;
        const float centerY = MATRIX_CENTER_Y;

        // Center circle
        circles.push_back({centerX, centerY, baseRadius});

        // First ring - 6 circles around the center
        for (int i = 0; i < 6; i++)
        {
            const float angle = (2.0f * PI * i) / 6.0f;
            const float x = centerX + baseRadius * cos(angle);
            const float y = centerY + baseRadius * sin(angle);
            circles.push_back({x, y, baseRadius});
        }

        // Second ring - 6 more circles
        for (int i = 0; i < 6; i++)
        {
            const float angle = (2.0f * PI * i) / 6.0f + (PI / 6.0f);     // Offset by 30 degrees
            const float x = centerX + (baseRadius * 1.732f) * cos(angle); // 1.732 ≈ sqrt(3)
            const float y = centerY + (baseRadius * 1.732f) * sin(angle);
            circles.push_back({x, y, baseRadius});
        }

        // Third partial ring for fuller pattern
        for (int i = 0; i < 6; i++)
        {
            const float angle = (2.0f * PI * i) / 6.0f;
            const float x = centerX + (baseRadius * 2.0f) * cos(angle);
            const float y = centerY + (baseRadius * 2.0f) * sin(angle);
            circles.push_back({x, y, baseRadius});
        }

        hueShift = random8();
        palette = randomPalette();
    }

    void render() override
    {
        // Update color animations
        hueShift += 1;                          // Slow hue rotation
        brightnessWave = beatsin8(3, 100, 255); // Very slow brightness wave

        // Draw filled flower with overlap effects
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                const uint8_t overlapCount = getOverlapCount(x, y);

                if (overlapCount > 0)
                {
                    // Create petal effects based on overlap count
                    const uint8_t colorIndex = (overlapCount * 40 + hueShift) % 255;
                    uint8_t brightness = 0;

                    if (overlapCount == 1)
                    {
                        // Single circle area - dimmer
                        brightness = scale8(brightnessWave, 100);
                    }
                    else if (overlapCount == 2)
                    {
                        // Petal area - brighter
                        brightness = scale8(brightnessWave, 200);
                    }
                    else
                    {
                        // Center areas with more overlap - brightest
                        brightness = brightnessWave;
                    }

                    // Add some sparkle based on position and time
                    const uint8_t sparkle = inoise8(x * 10, y * 10, millis() / 10) > 200 ? 50 : 0;
                    brightness = qadd8(brightness, sparkle);

                    const CRGB color = ColorFromPalette(palette, colorIndex, brightness);
                    Gfx(x, y) = color;
                }
            }
        }

        // Draw circle outlines for definition
        const uint8_t outlineBrightness = scale8(brightnessWave, 150);
        for (const auto &[x, y, radius] : circles)
        {
            const uint8_t colorIndex = static_cast<uint8_t>(hueShift + (x + y) / 4) % 255;
            const CRGB outlineColor = ColorFromPalette(palette, colorIndex, outlineBrightness);
            drawCircle(x, y, radius, outlineColor);
        }
    }
};
