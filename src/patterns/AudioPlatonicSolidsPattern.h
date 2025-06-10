#pragma once

#include "Geometry.h"
#include "Pattern.h"

class AudioPlatonicSolidsPattern final : public Pattern
{
    static constexpr uint8_t MAX_VERTICES = 20;
    static constexpr uint8_t MAX_EDGES = 30;

    struct Solid3D
    {
        Vertex vertices[MAX_VERTICES];
        EdgePoint edges[MAX_EDGES];
        uint8_t vertexCount;
        uint8_t edgeCount;
    };

    Solid3D currentSolid;
    Point screenPoints[MAX_VERTICES];
    uint8_t angleX = 0;
    uint8_t angleY = 0;
    uint8_t angleZ = 0;
    uint8_t rotSpeedX = 1;
    uint8_t rotSpeedY = 2;
    uint8_t rotSpeedZ = 3;
    uint8_t colorOffset = 0;
    uint8_t solidType = 0;
    uint8_t zDistance = 120;
    uint8_t baseZoom = 64;

    void createTetrahedron()
    {
        // Tetrahedron - 4 vertices, 6 edges
        currentSolid.vertexCount = 4;
        currentSolid.edgeCount = 6;

        currentSolid.vertices[0].set(30, 30, 30);
        currentSolid.vertices[1].set(-30, -30, 30);
        currentSolid.vertices[2].set(-30, 30, -30);
        currentSolid.vertices[3].set(30, -30, -30);

        currentSolid.edges[0].set(0, 1);
        currentSolid.edges[1].set(0, 2);
        currentSolid.edges[2].set(0, 3);
        currentSolid.edges[3].set(1, 2);
        currentSolid.edges[4].set(1, 3);
        currentSolid.edges[5].set(2, 3);
    }

    void createCube()
    {
        // Cube - 8 vertices, 12 edges
        currentSolid.vertexCount = 8;
        currentSolid.edgeCount = 12;

        currentSolid.vertices[0].set(-25, 25, 25);
        currentSolid.vertices[1].set(25, 25, 25);
        currentSolid.vertices[2].set(25, -25, 25);
        currentSolid.vertices[3].set(-25, -25, 25);
        currentSolid.vertices[4].set(-25, 25, -25);
        currentSolid.vertices[5].set(25, 25, -25);
        currentSolid.vertices[6].set(25, -25, -25);
        currentSolid.vertices[7].set(-25, -25, -25);

        // 12 edges of cube
        currentSolid.edges[0].set(0, 1);
        currentSolid.edges[1].set(1, 2);
        currentSolid.edges[2].set(2, 3);
        currentSolid.edges[3].set(3, 0);
        currentSolid.edges[4].set(4, 5);
        currentSolid.edges[5].set(5, 6);
        currentSolid.edges[6].set(6, 7);
        currentSolid.edges[7].set(7, 4);
        currentSolid.edges[8].set(0, 4);
        currentSolid.edges[9].set(1, 5);
        currentSolid.edges[10].set(2, 6);
        currentSolid.edges[11].set(3, 7);
    }

    void createOctahedron()
    {
        // Octahedron - 6 vertices, 12 edges
        currentSolid.vertexCount = 6;
        currentSolid.edgeCount = 12;

        currentSolid.vertices[0].set(0, 35, 0);  // Top
        currentSolid.vertices[1].set(35, 0, 0);  // Right
        currentSolid.vertices[2].set(0, 0, 35);  // Front
        currentSolid.vertices[3].set(-35, 0, 0); // Left
        currentSolid.vertices[4].set(0, 0, -35); // Back
        currentSolid.vertices[5].set(0, -35, 0); // Bottom

        // Upper pyramid edges
        currentSolid.edges[0].set(0, 1);
        currentSolid.edges[1].set(0, 2);
        currentSolid.edges[2].set(0, 3);
        currentSolid.edges[3].set(0, 4);
        // Middle square edges
        currentSolid.edges[4].set(1, 2);
        currentSolid.edges[5].set(2, 3);
        currentSolid.edges[6].set(3, 4);
        currentSolid.edges[7].set(4, 1);
        // Lower pyramid edges
        currentSolid.edges[8].set(5, 1);
        currentSolid.edges[9].set(5, 2);
        currentSolid.edges[10].set(5, 3);
        currentSolid.edges[11].set(5, 4);
    }

    void createDodecahedron()
    {
        // Dodecahedron - 20 vertices, 30 edges
        currentSolid.vertexCount = 20;
        currentSolid.edgeCount = 30;

        // Golden ratio approximation: φ ≈ 1.618, use 26 (16 * 1.625)
        int16_t phi = 26;
        int16_t one = 16;

        // Cube vertices scaled
        currentSolid.vertices[0].set(one, one, one);
        currentSolid.vertices[1].set(one, one, -one);
        currentSolid.vertices[2].set(one, -one, one);
        currentSolid.vertices[3].set(one, -one, -one);
        currentSolid.vertices[4].set(-one, one, one);
        currentSolid.vertices[5].set(-one, one, -one);
        currentSolid.vertices[6].set(-one, -one, one);
        currentSolid.vertices[7].set(-one, -one, -one);

        // Face centers of cube
        currentSolid.vertices[8].set(0, phi, one / phi);
        currentSolid.vertices[9].set(0, phi, -one / phi);
        currentSolid.vertices[10].set(0, -phi, one / phi);
        currentSolid.vertices[11].set(0, -phi, -one / phi);
        currentSolid.vertices[12].set(one / phi, 0, phi);
        currentSolid.vertices[13].set(-one / phi, 0, phi);
        currentSolid.vertices[14].set(one / phi, 0, -phi);
        currentSolid.vertices[15].set(-one / phi, 0, -phi);
        currentSolid.vertices[16].set(phi, one / phi, 0);
        currentSolid.vertices[17].set(phi, -one / phi, 0);
        currentSolid.vertices[18].set(-phi, one / phi, 0);
        currentSolid.vertices[19].set(-phi, -one / phi, 0);

        // 30 edges (simplified selection)
        currentSolid.edges[0].set(0, 8);
        currentSolid.edges[1].set(0, 12);
        currentSolid.edges[2].set(0, 16);
        currentSolid.edges[3].set(1, 9);
        currentSolid.edges[4].set(1, 14);
        currentSolid.edges[5].set(1, 16);
        currentSolid.edges[6].set(2, 10);
        currentSolid.edges[7].set(2, 12);
        currentSolid.edges[8].set(2, 17);
        currentSolid.edges[9].set(3, 11);
        currentSolid.edges[10].set(3, 14);
        currentSolid.edges[11].set(3, 17);
        currentSolid.edges[12].set(4, 8);
        currentSolid.edges[13].set(4, 13);
        currentSolid.edges[14].set(4, 18);
        currentSolid.edges[15].set(5, 9);
        currentSolid.edges[16].set(5, 15);
        currentSolid.edges[17].set(5, 18);
        currentSolid.edges[18].set(6, 10);
        currentSolid.edges[19].set(6, 13);
        currentSolid.edges[20].set(6, 19);
        currentSolid.edges[21].set(7, 11);
        currentSolid.edges[22].set(7, 15);
        currentSolid.edges[23].set(7, 19);
        currentSolid.edges[24].set(8, 9);
        currentSolid.edges[25].set(10, 11);
        currentSolid.edges[26].set(12, 13);
        currentSolid.edges[27].set(14, 15);
        currentSolid.edges[28].set(16, 17);
        currentSolid.edges[29].set(18, 19);
    }

    void createIcosahedron()
    {
        // Icosahedron - 12 vertices, 30 edges
        currentSolid.vertexCount = 12;
        currentSolid.edgeCount = 30;

        // Golden ratio approximation
        int16_t phi = 26; // φ * 16
        int16_t one = 16;

        // 12 vertices of icosahedron
        currentSolid.vertices[0].set(0, one, phi);
        currentSolid.vertices[1].set(0, one, -phi);
        currentSolid.vertices[2].set(0, -one, phi);
        currentSolid.vertices[3].set(0, -one, -phi);
        currentSolid.vertices[4].set(one, phi, 0);
        currentSolid.vertices[5].set(one, -phi, 0);
        currentSolid.vertices[6].set(-one, phi, 0);
        currentSolid.vertices[7].set(-one, -phi, 0);
        currentSolid.vertices[8].set(phi, 0, one);
        currentSolid.vertices[9].set(phi, 0, -one);
        currentSolid.vertices[10].set(-phi, 0, one);
        currentSolid.vertices[11].set(-phi, 0, -one);

        // 30 edges connecting the vertices
        currentSolid.edges[0].set(0, 2);
        currentSolid.edges[1].set(0, 4);
        currentSolid.edges[2].set(0, 6);
        currentSolid.edges[3].set(0, 8);
        currentSolid.edges[4].set(0, 10);
        currentSolid.edges[5].set(1, 3);
        currentSolid.edges[6].set(1, 4);
        currentSolid.edges[7].set(1, 6);
        currentSolid.edges[8].set(1, 9);
        currentSolid.edges[9].set(1, 11);
        currentSolid.edges[10].set(2, 5);
        currentSolid.edges[11].set(2, 7);
        currentSolid.edges[12].set(2, 8);
        currentSolid.edges[13].set(2, 10);
        currentSolid.edges[14].set(3, 5);
        currentSolid.edges[15].set(3, 7);
        currentSolid.edges[16].set(3, 9);
        currentSolid.edges[17].set(3, 11);
        currentSolid.edges[18].set(4, 6);
        currentSolid.edges[19].set(4, 8);
        currentSolid.edges[20].set(4, 9);
        currentSolid.edges[21].set(5, 7);
        currentSolid.edges[22].set(5, 8);
        currentSolid.edges[23].set(5, 9);
        currentSolid.edges[24].set(6, 10);
        currentSolid.edges[25].set(6, 11);
        currentSolid.edges[26].set(7, 10);
        currentSolid.edges[27].set(7, 11);
        currentSolid.edges[28].set(8, 9);
        currentSolid.edges[29].set(10, 11);
    }

  public:
    static constexpr auto ID = "Platonic Solids";

    AudioPlatonicSolidsPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        solidType = random8(5); // 0=Tetrahedron, 1=Cube, 2=Octahedron, 3=Dodecahedron, 4=Icosahedron
        rotSpeedX = random8(1, 4);
        rotSpeedY = random8(1, 4);
        rotSpeedZ = random8(1, 4);
        zDistance = random8(100, 140);
        baseZoom = random8(50, 80);
        palette = randomPalette();

        switch (solidType)
        {
            case 0: createTetrahedron(); break;
            case 1: createCube(); break;
            case 2: createOctahedron(); break;
            case 3: createDodecahedron(); break;
            case 4: createIcosahedron(); break;
            default: createCube(); break;
        }
    }

    void render() override
    {
        // Beat effects
        if (Audio.isBeat && Audio.totalBeats % 4 == 0)
        {
            solidType = random8(5);
            switch (solidType)
            {
                case 0: createTetrahedron(); break;
                case 1: createCube(); break;
                case 2: createOctahedron(); break;
                case 3: createDodecahedron(); break;
                case 4: createIcosahedron(); break;
            }
        }

        // Fade background
        Gfx.dim(230);

        // Update rotation angles
        angleX += rotSpeedX + (Audio.energy8 >> 7);
        angleY += rotSpeedY + (Audio.energy8 >> 7);
        angleZ += rotSpeedZ + (Audio.energy8 >> 7);
        colorOffset += 2;

        // Calculate audio-reactive zoom
        uint8_t zoom = map(Audio.energy8Scaled, 0, 255, 40, 175); // Base zoom + audio boost

        // Rotate and project vertices
        for (uint8_t i = 0; i < currentSolid.vertexCount; i++)
        {
            // 3D rotation using sin8/cos8 - convert to integer math
            Vertex &v = currentSolid.vertices[i];
            int16_t vx = (int16_t)v.x;
            int16_t vy = (int16_t)v.y;
            int16_t vz = (int16_t)v.z;

            // Rotate around X axis
            int16_t cosX = cos8(angleX) - 128; // Convert to signed
            int16_t sinX = sin8(angleX) - 128;
            int16_t y1 = (vy * cosX - vz * sinX) >> 7;
            int16_t z1 = (vy * sinX + vz * cosX) >> 7;

            // Rotate around Y axis
            int16_t cosY = cos8(angleY) - 128;
            int16_t sinY = sin8(angleY) - 128;
            int16_t x2 = (vx * cosY + z1 * sinY) >> 7;
            int16_t z2 = (-vx * sinY + z1 * cosY) >> 7;

            // Rotate around Z axis
            int16_t cosZ = cos8(angleZ) - 128;
            int16_t sinZ = sin8(angleZ) - 128;
            int16_t x3 = (x2 * cosZ - y1 * sinZ) >> 7;
            int16_t y3 = (x2 * sinZ + y1 * cosZ) >> 7;

            // Perspective projection with audio-reactive zoom
            int16_t projZ = z2 + zDistance;
            screenPoints[i].x = MATRIX_CENTER_X + (x3 * zoom) / (projZ + 1);
            screenPoints[i].y = MATRIX_CENTER_Y - (y3 * zoom) / (projZ + 1);
        }

        // Draw edges with audio-reactive colors
        for (uint8_t i = 0; i < currentSolid.edgeCount; i++)
        {
            EdgePoint &edge = currentSolid.edges[i];
            Point &p1 = screenPoints[edge.x];
            Point &p2 = screenPoints[edge.y];

            uint8_t hue = colorOffset + (i << 4) + (Audio.energy8 >> 3);
            uint8_t brightness = 150 + (Audio.energy8 >> 2);

            Gfx.drawLine(p1.x, p1.y, p2.x, p2.y, ColorFromPalette(palette, hue, brightness));
        }
    }
};
