#pragma once

#include "Pattern.h"

class AudioAtomicPattern final : public Pattern
{
    static constexpr uint8_t MAX_ELECTRONS = 26;
    static constexpr uint8_t MAX_ORBITALS = 6;

    struct Electron
    {
        uint8_t orbital; // Which orbital ring (0-2)
        uint8_t angle;   // Current angle around orbit
        uint8_t speed;   // Orbital speed
        uint8_t hue;     // Color
        bool active;
    };

    struct Orbital
    {
        uint8_t radius;   // Orbit radius
        uint8_t tilt;     // 3D tilt angle
        uint8_t rotation; // Ring rotation
    };

    Electron electrons[MAX_ELECTRONS];
    Orbital orbitals[MAX_ORBITALS];
    uint8_t nucleusSize = 3;
    uint8_t colorOffset = 0;
    uint8_t activeElectrons = 4;

  public:
    static constexpr auto ID = "Atomic Structure";

    AudioAtomicPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {

        activeElectrons = random8(16, MAX_ELECTRONS + 1);

        // Setup orbitals with varying sizes to fill screen
        for (uint8_t i = 0; i < MAX_ORBITALS; i++)
        {
            orbitals[i].radius = 6 + (i * 6) + random8(4);
            orbitals[i].tilt = random8();
            orbitals[i].rotation = random8();
        }

        // Setup electrons
        for (uint8_t i = 0; i < MAX_ELECTRONS; i++)
        {
            electrons[i].active = (i < activeElectrons);
            electrons[i].orbital = i % MAX_ORBITALS;
            electrons[i].angle = random8();
            electrons[i].speed = random8(1, 4);
            electrons[i].hue = random8();
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
            activeElectrons = random8(12, MAX_ELECTRONS + 1);
            for (uint8_t i = 0; i < MAX_ELECTRONS; i++)
            {
                electrons[i].active = (i < activeElectrons);
            }
        }

        // Fade background
        Gfx.dim(220);

        // Update colors and orbital rotations
        colorOffset += 2;
        for (uint8_t i = 0; i < MAX_ORBITALS; i++)
        {
            orbitals[i].rotation += 1 + (Audio.energy8 >> 7);
        }

        // Draw nucleus with audio-reactive size
        uint8_t currentNucleusSize = nucleusSize + (Audio.energy8 >> 6);
        for (int8_t dx = -currentNucleusSize; dx <= currentNucleusSize; dx++)
        {
            for (int8_t dy = -currentNucleusSize; dy <= currentNucleusSize; dy++)
            {
                uint8_t distance = abs(dx) + abs(dy);
                if (distance <= currentNucleusSize)
                {
                    uint8_t brightness = 255 - (distance * 200 / max<uint8_t>(currentNucleusSize + 1, 1));
                    int16_t x = MATRIX_CENTER_X + dx;
                    int16_t y = MATRIX_CENTER_Y + dy;
                    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                    {
                        Gfx(x, y) += ColorFromPalette(palette, colorOffset, brightness);
                    }
                }
            }
        }

        // Draw orbital paths and electrons
        for (uint8_t i = 0; i < MAX_ELECTRONS; i++)
        {
            if (!electrons[i].active)
                continue;

            Orbital &orbit = orbitals[electrons[i].orbital];

            // Update electron position
            electrons[i].angle += electrons[i].speed + (Audio.energy8 >> 6);

            // 3D orbital calculation
            uint8_t tiltedAngle = electrons[i].angle + orbit.rotation;
            int16_t x = orbit.radius * (cos8(electrons[i].angle) - 128) >> 7;
            int16_t y = orbit.radius * (sin8(electrons[i].angle) - 128) >> 7;

            // Apply orbital tilt
            int16_t z = y * (sin8(orbit.tilt) - 128) >> 7;
            y = y * (cos8(orbit.tilt) - 128) >> 7;

            // Perspective projection
            int16_t screenX = MATRIX_CENTER_X + (x * 100) / (z + 120);
            int16_t screenY = MATRIX_CENTER_Y + (y * 100) / (z + 120);

            // Draw electron
            if (screenX >= 0 && screenX < MATRIX_WIDTH && screenY >= 0 && screenY < MATRIX_HEIGHT)
            {
                uint8_t brightness = 200 + (Audio.energy8 >> 3);
                uint8_t hue = electrons[i].hue + colorOffset;
                Gfx(screenX, screenY) += ColorFromPalette(palette, hue, brightness);

                // Add glow around electron
                for (int8_t gx = -1; gx <= 1; gx++)
                {
                    for (int8_t gy = -1; gy <= 1; gy++)
                    {
                        int16_t glowX = screenX + gx;
                        int16_t glowY = screenY + gy;
                        if (glowX >= 0 && glowX < MATRIX_WIDTH && glowY >= 0 && glowY < MATRIX_HEIGHT)
                        {
                            Gfx(glowX, glowY) += ColorFromPalette(palette, hue + 64, brightness >> 2);
                        }
                    }
                }
            }
        }

        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
};
