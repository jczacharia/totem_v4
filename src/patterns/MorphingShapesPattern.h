#pragma once

class MorphingShapesPattern final : public Pattern
{
    static constexpr uint8_t MAX_VERTICES = 8;

    struct Vertex
    {
        int16_t x;
        int16_t y;
    };

    Vertex currentShape[MAX_VERTICES]{};
    Vertex targetShape[MAX_VERTICES]{};
    uint8_t numVertices = 3;
    uint8_t targetNumVertices = 3;
    uint8_t morphProgress = 0;
    uint8_t morphSpeed = 10;
    uint16_t rotation = 0;
    uint16_t rotationSpeed = 200;
    uint8_t scale = 128;
    uint8_t targetScale = 128;
    uint8_t beatBrightness = 0;
    uint8_t kaleidoscopeMode = 0;
    CRGBPalette16 palette = randomPalette();

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

    void generateShape(Vertex *shape, const ShapeType type, uint8_t &vertCount) const
    {
        const int16_t radius = 20 * Audio.energy8Scaled + 1;

        switch (type)
        {
            case TRIANGLE:
                vertCount = 3;
                for (uint8_t i = 0; i < 3; i++)
                {
                    const uint16_t angle = (i * 65536 / 3) + 16384;
                    shape[i].x = (cos16(angle) * radius) >> 16;
                    shape[i].y = (sin16(angle) * radius) >> 16;
                }
                break;

            case SQUARE:
                vertCount = 4;
                for (uint8_t i = 0; i < 4; i++)
                {
                    const uint16_t angle = (i * 65536 / 4) + 8192;
                    shape[i].x = (cos16(angle) * radius) >> 16;
                    shape[i].y = (sin16(angle) * radius) >> 16;
                }
                break;

            case PENTAGON:
                vertCount = 5;
                for (uint8_t i = 0; i < 5; i++)
                {
                    const uint16_t angle = (i * 65536 / 5) + 13107;
                    shape[i].x = (cos16(angle) * radius) >> 16;
                    shape[i].y = (sin16(angle) * radius) >> 16;
                }
                break;

            case HEXAGON:
                vertCount = 6;
                for (uint8_t i = 0; i < 6; i++)
                {
                    const uint16_t angle = i * 65536 / 6;
                    shape[i].x = (cos16(angle) * radius) >> 16;
                    shape[i].y = (sin16(angle) * radius) >> 16;
                }
                break;

            case STAR:
                vertCount = 8;
                for (uint8_t i = 0; i < 8; i++)
                {
                    const uint16_t angle = (i * 65536 / 8);
                    const int16_t r = (i & 1) ? radius : radius / 2;
                    shape[i].x = (cos16(angle) * r) >> 16;
                    shape[i].y = (sin16(angle) * r) >> 16;
                }
                break;
            default: break;
        }
    }

    void drawShape(
        const Vertex *shape,
        const uint8_t verts,
        const int16_t centerX,
        const int16_t centerY,
        const uint8_t brightness,
        const uint8_t currentScale,
        const uint16_t currentRotation)
    {
        if (verts < 2)
            return;

        for (uint8_t i = 0; i < verts; i++)
        {
            const uint8_t next = (i + 1) % verts;

            int32_t x1 = shape[i].x;
            int32_t y1 = shape[i].y;
            int32_t x2 = shape[next].x;
            int32_t y2 = shape[next].y;

            x1 = (x1 * currentScale) >> 5;
            y1 = (y1 * currentScale) >> 5;
            x2 = (x2 * currentScale) >> 5;
            y2 = (y2 * currentScale) >> 5;

            const int16_t cos_r = cos16(currentRotation) >> 8;
            const int16_t sin_r = sin16(currentRotation) >> 8;

            const int16_t rx1 = ((x1 * cos_r - y1 * sin_r) >> 8) + centerX;
            const int16_t ry1 = ((x1 * sin_r + y1 * cos_r) >> 8) + centerY;
            const int16_t rx2 = ((x2 * cos_r - y2 * sin_r) >> 8) + centerX;
            const int16_t ry2 = ((x2 * sin_r + y2 * cos_r) >> 8) + centerY;

            const CRGB color = ColorFromPalette(palette, i * (256 / verts), brightness);
            Gfx.drawLine(rx1 >> 8, ry1 >> 8, rx2 >> 8, ry2 >> 8, color);
        }
    }

  public:
    static constexpr auto ID = "Morphing Shapes";

    explicit MorphingShapesPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        rotation = 0;
        scale = 128;
        targetScale = 128;
        morphProgress = 255;
        beatBrightness = 0;
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
        currentShapeType = static_cast<ShapeType>(random8(SHAPE_COUNT));
        generateShape(currentShape, currentShapeType, numVertices);
        generateShape(targetShape, currentShapeType, targetNumVertices);
        palette = randomPalette();
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                beatBrightness = 255;
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);

                if (Audio.energy8Scaled >= 254)
                {
                    if (const auto newShape = static_cast<ShapeType>(random8(SHAPE_COUNT));
                        newShape != currentShapeType)
                    {
                        currentShapeType = newShape;
                        generateShape(targetShape, currentShapeType, targetNumVertices);
                        morphProgress = 0;
                        morphSpeed = 5 + (static_cast<uint8_t>(Audio.energy64fScaled) >> 2);
                    }
                }

                if (random8() < 100)
                {
                    rotationSpeed = 100 + random16(0, 400);
                    if (random8() < 128)
                    {
                        rotationSpeed = -rotationSpeed;
                    }
                }

                targetScale = 100 + (static_cast<uint8_t>(Audio.energy64fScaled) >> 1);
            }
        }

        if (morphProgress < 255)
        {
            morphProgress = qadd8(morphProgress, morphSpeed);

            const uint8_t maxVertices = max(numVertices, targetNumVertices);
            for (uint8_t i = 0; i < maxVertices; i++)
            {
                if (i < numVertices && i < targetNumVertices)
                {
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

        const uint16_t rotDelta = rotationSpeed + (static_cast<uint8_t>(Audio.energy64f) << 1);
        rotation += rotDelta;

        if (scale < targetScale)
        {
            scale = qadd8(scale, 4);
        }
        else if (scale > targetScale)
        {
            scale = qsub8(scale, 4);
        }

        beatBrightness = scale8(beatBrightness, 230);

        drawShape(currentShape, numVertices, MATRIX_CENTER_X << 8, MATRIX_CENTER_Y << 8, 255, scale, rotation);

        for (uint8_t i = 0; i < 8; i++)
        {
            const uint8_t freqHeight = Audio.heights8[i * 8];
            const uint16_t angle = (i * 65536 / 8) + rotation;
            const int16_t x = MATRIX_CENTER_X + ((cos16(angle) >> 8) * 20 >> 8);
            const int16_t y = MATRIX_CENTER_Y + ((sin16(angle) >> 8) * 20 >> 8);

            const uint8_t miniScale = 32 + (freqHeight >> 1);
            drawShape(currentShape, numVertices, x << 8, y << 8, Audio.energy8Scaled, miniScale, rotation * 2);
        }

        Gfx.randomKaleidoscope(kaleidoscopeMode);
        if (Audio.totalBeats % 3 == 0)
        {
            Gfx.kaleidoscope2();
        }
        else
        {
            Gfx.kaleidoscope1();
        }
    }
};
