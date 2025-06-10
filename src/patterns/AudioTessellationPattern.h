#pragma once

#include "Pattern.h"
#include "Util.h"

class AudioTessellationPattern final : public Pattern
{
    struct Polygon
    {
        uint8_t x, y;        // Center position
        uint8_t size;        // Current size
        uint8_t baseSize;    // Base size
        uint8_t sides;       // Number of sides (3-8)
        uint8_t rotation;    // Current rotation
        uint8_t audioBin;    // Which audio bin drives this polygon
        uint8_t colorOffset; // Color offset
        uint8_t pulsePhase;  // Individual pulse phase
    };

    static constexpr uint8_t MAX_POLYGONS = 12;
    Polygon polygons[MAX_POLYGONS];
    uint8_t activePolygons = 8;
    uint8_t globalRotation = 0;
    uint8_t morphPhase = 0;
    uint8_t tessellationGrid = 8;
    bool morphMode = false;
    uint8_t rotationSpeed = 2;

  public:
    static constexpr auto ID = "Tessellation";

    AudioTessellationPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        activePolygons = random8(6, MAX_POLYGONS + 1);
        tessellationGrid = random8(6, 12);
        rotationSpeed = random8(1, 4);
        morphMode = random8(2);

        // Initialize polygons in a grid pattern
        uint8_t spacing = max(MATRIX_WIDTH, MATRIX_HEIGHT) / tessellationGrid;
        uint8_t polyIndex = 0;

        for (uint8_t row = 0; row < tessellationGrid && polyIndex < activePolygons; row++)
        {
            for (uint8_t col = 0; col < tessellationGrid && polyIndex < activePolygons; col++)
            {
                polygons[polyIndex].x = col * spacing + (spacing >> 1);
                polygons[polyIndex].y = row * spacing + (spacing >> 1);
                polygons[polyIndex].baseSize = random8(3, 8);
                polygons[polyIndex].size = polygons[polyIndex].baseSize;
                polygons[polyIndex].sides = random8(3, 9);
                polygons[polyIndex].rotation = random8();
                polygons[polyIndex].audioBin = (polyIndex * BINS / activePolygons) % BINS;
                polygons[polyIndex].colorOffset = polyIndex * (255 / activePolygons);
                polygons[polyIndex].pulsePhase = random8();
                polyIndex++;
            }
        }

        palette = randomPalette();
        kaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        // Beat effects
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                // Randomize polygon properties
                for (uint8_t i = 0; i < activePolygons; i++)
                {
                    polygons[i].sides = random8(3, 9);
                    polygons[i].baseSize = random8(3, 10);
                }
                morphMode = !morphMode;
                kaleidoscope = random8(2);
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
            }
        }

        globalRotation += rotationSpeed;
        morphPhase += 3;

        // Fade for trails
        Gfx.dim(200);

        // Update and draw polygons
        for (uint8_t i = 0; i < activePolygons; i++)
        {
            updatePolygon(i);
            drawPolygon(i);
        }

        // Draw morphing connections between nearby polygons
        if (morphMode && Audio.energy8 > 100)
        {
            drawMorphConnections();
        }

        Gfx.randomKaleidoscope(kaleidoscopeMode);
        if (kaleidoscope)
            Gfx.kaleidoscope1();
        else
            Gfx.kaleidoscope2();
    }

  private:
    void updatePolygon(uint8_t index)
    {
        Polygon &poly = polygons[index];

        // Update rotation - each polygon rotates at different speeds
        poly.rotation += rotationSpeed + (index >> 1);

        // Update pulse phase
        poly.pulsePhase += 4 + (index >> 2);

        // Audio-reactive size changes
        uint8_t audioHeight = Audio.heights8[poly.audioBin];
        poly.size = poly.baseSize + (audioHeight >> 4);
        if (poly.size > 15)
            poly.size = 15;

        // Move polygons slightly with audio energy
        if (Audio.energy8 > 150)
        {
            int8_t moveX = (sin8(globalRotation + (index * 32)) - 128) >> 5;
            int8_t moveY = (cos8(globalRotation + (index * 45)) - 128) >> 5;
            poly.x = (poly.x + moveX) % MATRIX_WIDTH;
            poly.y = (poly.y + moveY) % MATRIX_HEIGHT;
        }

        // Morph sides based on audio
        if (morphMode)
        {
            uint8_t morphAmount = Audio.energy8 >> 5;
            if (morphAmount > 0 && random8(20) == 0)
            {
                poly.sides = 3 + (morphAmount % 6);
            }
        }
    }

    void drawPolygon(uint8_t index)
    {
        const Polygon &poly = polygons[index];

        // Calculate audio-reactive brightness
        uint8_t audioHeight = Audio.heights8[poly.audioBin];
        uint8_t baseBrightness = 60 + (audioHeight >> 3);

        // Add pulse effect
        uint8_t pulse = sin8(poly.pulsePhase) >> 2;
        uint8_t brightness = baseBrightness + pulse;

        // Calculate color with rotation and audio influence
        uint8_t hue = poly.colorOffset + globalRotation + (audioHeight >> 2);

        CRGB color = ColorFromPalette(palette, hue, brightness);

        // Draw polygon using line segments
        drawPolygonShape(poly.x, poly.y, poly.size, poly.sides, poly.rotation + globalRotation, color);

        // Draw center dot for stronger audio
        if (audioHeight > 120)
        {
            Gfx(poly.x, poly.y) += ColorFromPalette(palette, hue + 64, 255);
        }
    }

    void drawPolygonShape(uint8_t centerX, uint8_t centerY, uint8_t radius, uint8_t sides, uint8_t rotation, CRGB color)
    {
        if (sides < 3)
            sides = 3;

        uint8_t angleStep = 255 / sides;

        // Draw filled polygon by connecting center to vertices
        for (uint8_t i = 0; i < sides; i++)
        {
            uint8_t angle1 = rotation + (i * angleStep);
            uint8_t angle2 = rotation + ((i + 1) * angleStep);

            // Calculate vertices
            int8_t x1 = centerX + ((radius * (cos8(angle1) - 128)) >> 7);
            int8_t y1 = centerY + ((radius * (sin8(angle1) - 128)) >> 7);
            int8_t x2 = centerX + ((radius * (cos8(angle2) - 128)) >> 7);
            int8_t y2 = centerY + ((radius * (sin8(angle2) - 128)) >> 7);

            // Draw triangle from center to edge
            drawTriangleFilled(centerX, centerY, x1, y1, x2, y2, color);
        }
    }

    void drawTriangleFilled(int8_t x0, int8_t y0, int8_t x1, int8_t y1, int8_t x2, int8_t y2, CRGB color)
    {
        // Simple triangle fill using line drawing
        Gfx.drawLine(x0, y0, x1, y1, color);
        Gfx.drawLine(x1, y1, x2, y2, color);
        Gfx.drawLine(x2, y2, x0, y0, color);

        // Fill center area with dimmer version
        CRGB fillColor = color;
        fillColor.nscale8(128);

        // Calculate triangle center and fill a small area
        int8_t centerX = (x0 + x1 + x2) / 3;
        int8_t centerY = (y0 + y1 + y2) / 3;

        if (centerX >= 0 && centerX < MATRIX_WIDTH && centerY >= 0 && centerY < MATRIX_HEIGHT)
        {
            Gfx(centerX, centerY) += fillColor;
        }
    }

    void drawMorphConnections()
    {
        // Draw connections between nearby polygons based on audio
        for (uint8_t i = 0; i < activePolygons; i++)
        {
            for (uint8_t j = i + 1; j < activePolygons; j++)
            {
                // Calculate distance between polygons
                int16_t dx = polygons[i].x - polygons[j].x;
                int16_t dy = polygons[i].y - polygons[j].y;
                uint8_t distance = sqrt16(dx * dx + dy * dy);

                // Draw connection if they're close and audio is strong
                if (distance < 20)
                {
                    uint8_t connectionStrength = (Audio.heights8[i % BINS] + Audio.heights8[j % BINS]) >> 1;
                    if (connectionStrength > 80)
                    {
                        uint8_t brightness = connectionStrength >> 2;
                        uint8_t hue = morphPhase + (i * 16) + (j * 24);

                        // Draw morphing connection line
                        CRGB connectionColor = ColorFromPalette(palette, hue, brightness);
                        connectionColor.nscale8(64); // Make connections subtle

                        Gfx.drawLine(polygons[i].x, polygons[i].y, polygons[j].x, polygons[j].y, connectionColor);

                        // Add sparkles along the connection
                        if (connectionStrength > 150)
                        {
                            uint8_t midX = (polygons[i].x + polygons[j].x) >> 1;
                            uint8_t midY = (polygons[i].y + polygons[j].y) >> 1;
                            Gfx(midX, midY) += ColorFromPalette(palette, hue + 128, 255);
                        }
                    }
                }
            }
        }
    }
};
