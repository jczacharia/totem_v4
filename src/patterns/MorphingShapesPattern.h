#pragma once

#include "Matrix.h"

class MorphingShapesPattern final : public Pattern
{
    // Shape vertex storage (using integer coordinates scaled by 256 for precision)
    static constexpr uint8_t MAX_VERTICES = 8;

    struct Vertex
    {
        int16_t x; // Fixed point 8.8 format (256 = 1.0)
        int16_t y;
    };

    // Current and target shapes
    Vertex currentShape[MAX_VERTICES];
    Vertex targetShape[MAX_VERTICES];
    uint8_t numVertices = 3;
    uint8_t targetNumVertices = 3;

    // Morphing parameters
    uint8_t morphProgress = 0; // 0-255
    uint8_t morphSpeed = 10;

    // Rotation and scaling (using fixed point)
    uint16_t rotation = 0; // 0-65535 represents 0-360 degrees
    uint16_t rotationSpeed = 200;
    uint8_t scale = 128; // 128 = 1.0x scale
    uint8_t targetScale = 128;

    // Colors
    uint8_t beatBrightness = 0;
    uint8_t kaleidoscopeMode = 0;

    // Shape types
    enum ShapeType : uint8_t
    {
        TRIANGLE = 0,
        SQUARE,
        PENTAGON,
        HEXAGON,
        STAR,
        SHAPE_COUNT
    };

    ShapeType currentShapeType = TRIANGLE;

    void generateShape(Vertex* shape, ShapeType type, uint8_t& vertCount)
    {
        const int16_t radius = 20 * 256; // 20 pixels in fixed point

        switch (type)
        {
        case TRIANGLE:
            vertCount = 3;
            for (uint8_t i = 0; i < 3; i++)
            {
                uint16_t angle = (i * 65536 / 3) + 16384; // Start at top
                shape[i].x = (cos16(angle) * radius) >> 16;
                shape[i].y = (sin16(angle) * radius) >> 16;
            }
            break;

        case SQUARE:
            vertCount = 4;
            for (uint8_t i = 0; i < 4; i++)
            {
                uint16_t angle = (i * 65536 / 4) + 8192; // 45 degree offset
                shape[i].x = (cos16(angle) * radius) >> 16;
                shape[i].y = (sin16(angle) * radius) >> 16;
            }
            break;

        case PENTAGON:
            vertCount = 5;
            for (uint8_t i = 0; i < 5; i++)
            {
                uint16_t angle = (i * 65536 / 5) + 13107; // Start at top
                shape[i].x = (cos16(angle) * radius) >> 16;
                shape[i].y = (sin16(angle) * radius) >> 16;
            }
            break;

        case HEXAGON:
            vertCount = 6;
            for (uint8_t i = 0; i < 6; i++)
            {
                uint16_t angle = i * 65536 / 6;
                shape[i].x = (cos16(angle) * radius) >> 16;
                shape[i].y = (sin16(angle) * radius) >> 16;
            }
            break;

        case STAR:
            vertCount = 8; // 4 outer points, 4 inner points
            for (uint8_t i = 0; i < 8; i++)
            {
                uint16_t angle = (i * 65536 / 8);
                int16_t r = (i & 1) ? radius : radius / 2; // Alternate radius
                shape[i].x = (cos16(angle) * r) >> 16;
                shape[i].y = (sin16(angle) * r) >> 16;
            }
            break;
        }
    }

    void drawShape(const Vertex* shape, uint8_t verts, int16_t centerX, int16_t centerY,
                   uint8_t brightness, uint8_t currentScale, uint16_t currentRotation,
                   PatternContext& ctx)
    {
        if (verts < 2) return;

        // Draw lines between vertices
        for (uint8_t i = 0; i < verts; i++)
        {
            uint8_t next = (i + 1) % verts;

            // Apply rotation and scaling using integer math
            int32_t x1 = shape[i].x;
            int32_t y1 = shape[i].y;
            int32_t x2 = shape[next].x;
            int32_t y2 = shape[next].y;

            // Scale
            x1 = (x1 * currentScale) >> 5;
            y1 = (y1 * currentScale) >> 5;
            x2 = (x2 * currentScale) >> 5;
            y2 = (y2 * currentScale) >> 5;

            // Rotate using FastLED's sin16/cos16
            int16_t cos_r = cos16(currentRotation) >> 8;
            int16_t sin_r = sin16(currentRotation) >> 8;

            int16_t rx1 = ((x1 * cos_r - y1 * sin_r) >> 8) + centerX;
            int16_t ry1 = ((x1 * sin_r + y1 * cos_r) >> 8) + centerY;
            int16_t rx2 = ((x2 * cos_r - y2 * sin_r) >> 8) + centerX;
            int16_t ry2 = ((x2 * sin_r + y2 * cos_r) >> 8) + centerY;

            // Draw the line
            CRGB color = ColorFromPalette(ctx.currentPalette, i * (256 / verts), brightness);
            ctx.drawLine(rx1 >> 8, ry1 >> 8, rx2 >> 8, ry2 >> 8, color);
        }
    }

public:
    static constexpr auto ID = "MorphingShapes";

    MorphingShapesPattern() : Pattern(ID)
    {
    }

    void start(PatternContext& ctx) override
    {
        rotation = 0;
        scale = 128;
        targetScale = 128;
        morphProgress = 255;
        beatBrightness = 0;
        kaleidoscopeMode = random8(0, 4);

        currentShapeType = (ShapeType)random8(SHAPE_COUNT);
        generateShape(currentShape, currentShapeType, numVertices);
        generateShape(targetShape, currentShapeType, targetNumVertices);
    }

    void render(PatternContext& ctx) override
    {
        // Handle beat
        if (ctx.audio.isBeat)
        {
            beatBrightness = 255;
            kaleidoscopeMode = random8(0, 4);

            // Change shape on strong beats
            if (ctx.audio.energy64fScaled > 40)
            {
                ShapeType newShape = (ShapeType)random8(SHAPE_COUNT);
                if (newShape != currentShapeType)
                {
                    currentShapeType = newShape;
                    generateShape(targetShape, currentShapeType, targetNumVertices);
                    morphProgress = 0;
                    morphSpeed = 5 + ((uint8_t)ctx.audio.energy64fScaled >> 2); // Faster morph with higher energy
                }
            }

            // Random rotation change
            if (random8() < 100)
            {
                rotationSpeed = 100 + random16(0, 400);
                if (random8() < 128) rotationSpeed = -rotationSpeed;
            }

            // Scale pulse
            targetScale = 100 + ((uint8_t)ctx.audio.energy64fScaled >> 1);
        }

        // Update morphing
        if (morphProgress < 255)
        {
            morphProgress = qadd8(morphProgress, morphSpeed);

            // Interpolate vertices
            uint8_t maxVerts = max(numVertices, targetNumVertices);
            for (uint8_t i = 0; i < maxVerts; i++)
            {
                if (i < numVertices && i < targetNumVertices)
                {
                    // Lerp between vertices
                    currentShape[i].x = lerp16by8(currentShape[i].x, targetShape[i].x, morphProgress);
                    currentShape[i].y = lerp16by8(currentShape[i].y, targetShape[i].y, morphProgress);
                }
            }

            if (morphProgress >= 250)
            {
                numVertices = targetNumVertices;
                memcpy(currentShape, targetShape, sizeof(Vertex) * numVertices);
            }
        }

        // Update rotation with audio influence
        uint16_t rotDelta = rotationSpeed + ((uint8_t)ctx.audio.energy64f << 1);
        rotation += rotDelta;

        // Update scale
        if (scale < targetScale)
            scale = qadd8(scale, 4);
        else if (scale > targetScale)
            scale = qsub8(scale, 4);

        // Decay beat brightness
        beatBrightness = scale8(beatBrightness, 230);


        // Draw main shape in center
        drawShape(currentShape, numVertices, MATRIX_CENTER_X << 8, MATRIX_CENTER_Y << 8,
                  255, scale, rotation, ctx);

        // Draw frequency-reactive mini shapes around the main one
        for (uint8_t i = 0; i < 8; i++)
        {
            uint8_t freqHeight = ctx.audio.heights8[i * 8];
            uint16_t angle = (i * 65536 / 8) + rotation;
            int16_t x = MATRIX_CENTER_X + ((cos16(angle) >> 8) * 20 >> 8);
            int16_t y = MATRIX_CENTER_Y + ((sin16(angle) >> 8) * 20 >> 8);

            uint8_t miniScale = 32 + (freqHeight >> 1);
            drawShape(currentShape, numVertices, x << 8, y << 8,
                      ctx.audio.energy8Scaled, miniScale, rotation * 2, ctx);
        }

        switch (kaleidoscopeMode)
        {
        case 0:
            ctx.kaleidoscope3();
            ctx.kaleidoscope1();
            break;
        case 1:
            ctx.kaleidoscope4();
            ctx.kaleidoscope1();
            break;
        case 2:
            ctx.kaleidoscope4();
            ctx.kaleidoscope2();
            break;
        default:
            ctx.kaleidoscope3();
            ctx.kaleidoscope2();
            break;
        }
    }
};
