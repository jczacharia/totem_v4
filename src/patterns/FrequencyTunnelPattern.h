#pragma once

#include "Pattern.h"

class FrequencyTunnelPattern final : public Pattern
{
    // Tunnel parameters
    uint8_t tunnelDepth;
    uint8_t ringCount;
    uint8_t centerX;
    uint8_t centerY;

    // Movement
    uint8_t zOffset;
    uint8_t zSpeed;
    uint8_t rotationAngle;
    uint8_t rotationSpeed;

    // Visual parameters
    uint8_t baseHue;
    uint8_t hueShift;
    uint8_t fadeAmount;
    uint8_t ringThickness;
    bool pulseOnBeat;
    bool rotateOnBeat;

    // Audio response
    uint8_t bassZoom;
    uint8_t lastBeatCount;
    uint8_t beatPulse;

  public:
    static constexpr auto ID = "Frequency Tunnel";

    FrequencyTunnelPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Initialize tunnel
        tunnelDepth = 8;
        ringCount = 12;
        centerX = MATRIX_CENTER_X;
        centerY = MATRIX_CENTER_Y;

        // Movement
        zOffset = 0;
        zSpeed = 2;
        rotationAngle = 0;
        rotationSpeed = 1;

        // Visual parameters
        baseHue = random8();
        hueShift = random8(2, 8);
        fadeAmount = 220;
        ringThickness = 2;
        pulseOnBeat = random8(2);
        rotateOnBeat = random8(2);

        // Audio
        bassZoom = 0;
        lastBeatCount = 0;
        beatPulse = 0;

        palette = randomPalette();
    }

    void render() override
    {
        // Fade for motion blur effect
        Gfx.dim(fadeAmount);

        // Update movement
        zOffset += zSpeed + (Audio.energy8Scaled >> 6);
        rotationAngle += rotationSpeed;

        // Beat detection
        if (Audio.isBeat && Audio.totalBeats != lastBeatCount)
        {
            lastBeatCount = Audio.totalBeats;
            beatPulse = 255;

            if (rotateOnBeat)
            {
                rotationSpeed = -rotationSpeed; // Reverse rotation
            }

            // Occasionally randomize on beat
            if (Audio.totalBeats % 8 == 0)
            {
                baseHue = random8();
                hueShift = random8(2, 8);
            }
        }

        // Fade beat pulse
        if (beatPulse > 0)
        {
            beatPulse -= 8;
        }

        // Bass response for tunnel zoom
        bassZoom = (Audio.peaks8[0] + Audio.peaks8[1] + Audio.peaks8[2]) / 12;

        // Draw tunnel rings from back to front
        for (uint8_t ring = ringCount; ring > 0; ring--)
        {
            // Calculate ring properties
            uint8_t z = (ring * tunnelDepth + zOffset) & 0xFF;
            uint8_t radius = 4 + (z >> 3) - (bassZoom >> 2);

            // Skip if too small
            if (radius < 2)
                continue;

            // Audio modulation for this ring
            uint8_t audioIndex = (ring * 4) % BINS;
            uint8_t audioMod = Audio.heights8[audioIndex] >> 3;

            // Ring distortion based on frequency
            uint8_t distortion = audioMod >> 1;

            // Color for this ring
            uint8_t hue = baseHue + (ring * hueShift) + (z >> 2);
            uint8_t brightness = 255 - (z >> 1);

            // Add beat pulse brightness
            if (pulseOnBeat && beatPulse > 0)
            {
                brightness = qadd8(brightness, beatPulse >> 2);
            }

            CRGB color = ColorFromPalette(palette, hue, brightness);

            // Draw octagonal ring (8 points for simplicity)
            for (uint8_t point = 0; point < 8; point++)
            {
                uint8_t angle = (point * 32) + rotationAngle + (z >> 1);

                // Calculate point position with audio distortion
                uint8_t pointRadius = radius + (sin8(angle + audioMod) >> 5);
                uint8_t x = centerX + ((pointRadius * cos8(angle)) >> 7) - (pointRadius >> 1);
                uint8_t y = centerY + ((pointRadius * sin8(angle)) >> 7) - (pointRadius >> 1);

                // Draw thick point
                for (uint8_t t = 0; t < ringThickness; t++)
                {
                    Gfx(x + t, y) += color;
                    Gfx(x, y + t) += color;
                }

                // Connect to next point
                uint8_t nextAngle = ((point + 1) * 32) + rotationAngle + (z >> 1);
                uint8_t nextRadius = radius + (sin8(nextAngle + audioMod) >> 5);
                uint8_t nextX = centerX + ((nextRadius * cos8(nextAngle)) >> 7) - (nextRadius >> 1);
                uint8_t nextY = centerY + ((nextRadius * sin8(nextAngle)) >> 7) - (nextRadius >> 1);

                Gfx.drawLine(x, y, nextX, nextY, color);
            }
        }

        // Draw center vortex
        uint8_t vortexSize = 2 + (beatPulse >> 6);
        uint8_t vortexBright = 200 + (Audio.energy8Scaled >> 2);

        for (int8_t dx = -vortexSize; dx <= vortexSize; dx++)
        {
            for (int8_t dy = -vortexSize; dy <= vortexSize; dy++)
            {
                if (abs(dx) + abs(dy) <= vortexSize)
                {
                    Gfx(centerX + dx, centerY + dy) += ColorFromPalette(palette, baseHue + 128, vortexBright);
                }
            }
        }

        // Add some spectrum visualization at the edges
        for (uint8_t i = 0; i < 16; i++)
        {
            uint8_t h = Audio.heights8[i * 4] >> 5;
            if (h > 0)
            {
                // Top edge
                Gfx(i * 4, h) += CHSV(baseHue + i * 16, 255, 200);
                Gfx(i * 4 + 1, h) += CHSV(baseHue + i * 16, 255, 200);

                // Bottom edge
                Gfx(i * 4, MATRIX_HEIGHT - 1 - h) += CHSV(baseHue + i * 16 + 128, 255, 200);
                Gfx(i * 4 + 1, MATRIX_HEIGHT - 1 - h) += CHSV(baseHue + i * 16 + 128, 255, 200);
            }
        }
    }
};
