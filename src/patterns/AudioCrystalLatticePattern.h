#pragma once

#include "Pattern.h"

class AudioCrystalLatticePattern final : public Pattern
{
    static constexpr uint8_t LATTICE_SIZE = 5;
    static constexpr uint8_t MAX_NODES = LATTICE_SIZE * LATTICE_SIZE * LATTICE_SIZE;

    struct CrystalNode
    {
        int8_t x, y, z;     // 3D position in lattice
        uint8_t brightness; // Current brightness
        uint8_t hue;        // Color
        bool active;        // Whether node is visible
    };

    CrystalNode nodes[MAX_NODES];
    uint8_t rotationX = 0;
    uint8_t rotationY = 0;
    uint8_t rotationZ = 0;
    uint8_t colorOffset = 0;
    uint8_t nodeSize = 15;
    uint8_t activeNodes = MAX_NODES;

  public:
    static constexpr auto ID = "Crystal Lattice";

    AudioCrystalLatticePattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        nodeSize = random8(8, 12);
        activeNodes = random8(MAX_NODES / 2, MAX_NODES + 1);

        // Setup 3D lattice positions
        uint8_t index = 0;
        for (int8_t x = -(LATTICE_SIZE / 2); x <= LATTICE_SIZE / 2; x++)
        {
            for (int8_t y = -(LATTICE_SIZE / 2); y <= LATTICE_SIZE / 2; y++)
            {
                for (int8_t z = -(LATTICE_SIZE / 2); z <= LATTICE_SIZE / 2; z++)
                {
                    if (index < MAX_NODES)
                    {
                        nodes[index].x = x * nodeSize;
                        nodes[index].y = y * nodeSize;
                        nodes[index].z = z * nodeSize;
                        nodes[index].brightness = random8(100, 255);
                        nodes[index].hue = random8();
                        nodes[index].active = (index < activeNodes);
                        index++;
                    }
                }
            }
        }

        palette = randomPalette();
        kaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        // Beat effects
        if (Audio.isBeat && Audio.totalBeats % 4 == 0)
        {
            kaleidoscope = random8(2);
            kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
            activeNodes = random8(MAX_NODES / 2, MAX_NODES + 1);
            for (uint8_t i = 0; i < MAX_NODES; i++)
            {
                nodes[i].active = (i < activeNodes);
                if (random8() < 50)
                {
                    nodes[i].hue = random8();
                }
            }
        }

        // Fade background
        Gfx.dim(230);

        // Update rotations with audio reactivity
        rotationX += 2 + (Audio.energy8 >> 7);
        rotationY += 1 + (Audio.energy8 >> 6);
        rotationZ += 3 + (Audio.energy8 >> 8);
        colorOffset += 2;

        // Draw crystal lattice
        for (uint8_t i = 0; i < MAX_NODES; i++)
        {
            if (!nodes[i].active)
                continue;

            CrystalNode &node = nodes[i];

            // Apply 3D rotations
            int16_t x = node.x;
            int16_t y = node.y;
            int16_t z = node.z;

            // Rotate around X axis
            int16_t tempY = y;
            y = (y * (cos8(rotationX) - 128) - z * (sin8(rotationX) - 128)) >> 7;
            z = (tempY * (sin8(rotationX) - 128) + z * (cos8(rotationX) - 128)) >> 7;

            // Rotate around Y axis
            int16_t tempX = x;
            x = (x * (cos8(rotationY) - 128) + z * (sin8(rotationY) - 128)) >> 7;
            z = (-tempX * (sin8(rotationY) - 128) + z * (cos8(rotationY) - 128)) >> 7;

            // Rotate around Z axis
            tempX = x;
            x = (x * (cos8(rotationZ) - 128) - y * (sin8(rotationZ) - 128)) >> 7;
            y = (tempX * (sin8(rotationZ) - 128) + y * (cos8(rotationZ) - 128)) >> 7;

            // Perspective projection with wider field of view
            int16_t screenX = MATRIX_CENTER_X + (x * 140) / (z + 120);
            int16_t screenY = MATRIX_CENTER_Y + (y * 140) / (z + 120);

            // Draw node if in bounds
            if (screenX >= 0 && screenX < MATRIX_WIDTH && screenY >= 0 && screenY < MATRIX_HEIGHT)
            {
                // Audio-reactive brightness
                uint8_t brightness = node.brightness + (Audio.energy8 >> 3);
                brightness = min<uint8_t>(brightness, 255);

                uint8_t hue = node.hue + colorOffset + (z >> 2);
                CRGB color = ColorFromPalette(palette, hue, brightness);

                // Draw node with glow
                Gfx(screenX, screenY) += color;

                // Add crystalline glow pattern
                for (int8_t dx = -1; dx <= 1; dx++)
                {
                    for (int8_t dy = -1; dy <= 1; dy++)
                    {
                        if (dx == 0 && dy == 0)
                            continue;

                        int16_t glowX = screenX + dx;
                        int16_t glowY = screenY + dy;

                        if (glowX >= 0 && glowX < MATRIX_WIDTH && glowY >= 0 && glowY < MATRIX_HEIGHT)
                        {
                            uint8_t glowBrightness = brightness >> 2;
                            Gfx(glowX, glowY) += ColorFromPalette(palette, hue + 32, glowBrightness);
                        }
                    }
                }
            }

            // Draw connections to nearby nodes
            for (uint8_t j = i + 1; j < MAX_NODES; j++)
            {
                if (!nodes[j].active)
                    continue;

                // Check if nodes are adjacent in lattice
                int8_t dx = abs(nodes[i].x - nodes[j].x);
                int8_t dy = abs(nodes[i].y - nodes[j].y);
                int8_t dz = abs(nodes[i].z - nodes[j].z);

                bool adjacent = (dx == nodeSize && dy == 0 && dz == 0) || (dx == 0 && dy == nodeSize && dz == 0) ||
                                (dx == 0 && dy == 0 && dz == nodeSize);

                if (adjacent && random8() > 200) // Draw some connections randomly for effect
                {
                    // Calculate second node screen position
                    CrystalNode &node2 = nodes[j];

                    int16_t x2 = node2.x;
                    int16_t y2 = node2.y;
                    int16_t z2 = node2.z;

                    // Apply same rotations
                    int16_t tempY2 = y2;
                    y2 = (y2 * (cos8(rotationX) - 128) - z2 * (sin8(rotationX) - 128)) >> 7;
                    z2 = (tempY2 * (sin8(rotationX) - 128) + z2 * (cos8(rotationX) - 128)) >> 7;

                    int16_t tempX2 = x2;
                    x2 = (x2 * (cos8(rotationY) - 128) + z2 * (sin8(rotationY) - 128)) >> 7;
                    z2 = (-tempX2 * (sin8(rotationY) - 128) + z2 * (cos8(rotationY) - 128)) >> 7;

                    tempX2 = x2;
                    x2 = (x2 * (cos8(rotationZ) - 128) - y2 * (sin8(rotationZ) - 128)) >> 7;
                    y2 = (tempX2 * (sin8(rotationZ) - 128) + y2 * (cos8(rotationZ) - 128)) >> 7;

                    int16_t screenX2 = MATRIX_CENTER_X + (x2 * 140) / (z2 + 120);
                    int16_t screenY2 = MATRIX_CENTER_Y + (y2 * 140) / (z2 + 120);

                    // Draw connection line
                    CRGB lineColor = ColorFromPalette(palette, colorOffset + (i << 4), 60);
                    Gfx.drawLine(screenX, screenY, screenX2, screenY2, lineColor);
                }
            }
        }

        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
};
