#pragma once

#include "Pattern.h"

class AudioSpaceDebrisPattern final : public Pattern
{
    // Space object structure
    struct SpaceObject
    {
        uint8_t x, y, z;    // 3D position (z is depth)
        uint8_t type;       // 0=star, 1=asteroid, 2=debris
        uint8_t size;       // Object size
        uint8_t speed;      // Movement speed
        uint8_t hue;        // Color
        uint8_t audioIndex; // Which frequency band it responds to
    };

    static constexpr uint8_t MAX_OBJECTS = 40;
    SpaceObject objects[MAX_OBJECTS];

    // Visual parameters
    uint8_t starfieldDensity;
    uint8_t baseHue;
    uint8_t fadeAmount;
    uint8_t perspectiveScale;
    bool showTrails;
    bool pulseOnBeat;

    // Movement
    uint8_t cameraShakeX;
    uint8_t cameraShakeY;
    uint8_t warpSpeed;

    // Audio response
    uint8_t beatFlash;
    uint8_t lastBeatCount;

  public:
    static constexpr auto ID = "Audio Space Debris";

    AudioSpaceDebrisPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Initialize space objects
        for (uint8_t i = 0; i < MAX_OBJECTS; i++)
        {
            objects[i].x = random8(MATRIX_WIDTH);
            objects[i].y = random8(MATRIX_HEIGHT);
            objects[i].z = random8(255);
            objects[i].type = random8(3);
            objects[i].size = random8(1, 4);
            objects[i].speed = random8(1, 4);
            objects[i].hue = random8();
            objects[i].audioIndex = random8(BINS);
        }

        // Visual parameters
        starfieldDensity = random8(30, 50);
        baseHue = random8();
        fadeAmount = 230;
        perspectiveScale = 3;
        showTrails = random8(2);
        pulseOnBeat = random8(2);

        // Movement
        cameraShakeX = 0;
        cameraShakeY = 0;
        warpSpeed = 0;

        // Audio
        beatFlash = 0;
        lastBeatCount = 0;

        palette = randomPalette();
    }

    void render() override
    {
        // Fade for motion trails
        if (showTrails)
        {
            Gfx.dim(fadeAmount);
        }
        else
        {
            Gfx.clear();
        }

        // Beat detection
        if (Audio.isBeat && Audio.totalBeats != lastBeatCount)
        {
            lastBeatCount = Audio.totalBeats;
            beatFlash = 255;

            // Camera shake on strong beats
            if (Audio.energy8Scaled > 200)
            {
                cameraShakeX = random8(5) - 2;
                cameraShakeY = random8(5) - 2;
            }
        }

        // Decay effects
        if (beatFlash > 0)
            beatFlash -= 10;
        if (warpSpeed > 0)
            warpSpeed -= 2;
        if (cameraShakeX != 0)
            cameraShakeX = cameraShakeX > 0 ? cameraShakeX - 1 : cameraShakeX + 1;
        if (cameraShakeY != 0)
            cameraShakeY = cameraShakeY > 0 ? cameraShakeY - 1 : cameraShakeY + 1;

        // Update base hue slowly
        baseHue += Audio.energy8Scaled >> 7;

        // Draw and update space objects
        for (uint8_t i = 0; i < MAX_OBJECTS; i++)
        {
            SpaceObject &obj = objects[i];

            // Move object towards viewer
            uint8_t audioBoost = Audio.heights8[obj.audioIndex] >> 5;
            obj.z -= obj.speed + warpSpeed + audioBoost;

            // Reset if passed camera
            if (obj.z < 10)
            {
                obj.x = random8(MATRIX_WIDTH);
                obj.y = random8(MATRIX_HEIGHT);
                obj.z = 255;
                obj.type = random8(3);
                obj.hue = random8();
            }

            // Calculate screen position with perspective
            uint8_t screenX =
                MATRIX_CENTER_X + ((obj.x - MATRIX_CENTER_X) * perspectiveScale * 255 / obj.z) + cameraShakeX;
            uint8_t screenY =
                MATRIX_CENTER_Y + ((obj.y - MATRIX_CENTER_Y) * perspectiveScale * 255 / obj.z) + cameraShakeY;

            // Skip if off screen
            if (screenX >= MATRIX_WIDTH || screenY >= MATRIX_HEIGHT)
                continue;

            // Calculate size based on depth and audio
            uint8_t displaySize = obj.size * 255 / obj.z;
            if (pulseOnBeat && beatFlash > 0)
            {
                displaySize += beatFlash >> 6;
            }

            // Calculate brightness based on depth
            uint8_t brightness = 255 - (obj.z >> 1);
            brightness = qadd8(brightness, Audio.heights8[obj.audioIndex] >> 2);

            // Draw based on object type
            switch (obj.type)
            {
                case 0: // Star
                {
                    CRGB color = ColorFromPalette(palette, obj.hue + baseHue, brightness);
                    Gfx(screenX, screenY) += color;

                    // Star twinkle
                    if (displaySize > 0 && random8(10) < 3)
                    {
                        Gfx(screenX + 1, screenY) += color;
                        Gfx(screenX - 1, screenY) += color;
                        Gfx(screenX, screenY + 1) += color;
                        Gfx(screenX, screenY - 1) += color;
                    }
                }
                break;

                case 1: // Asteroid
                {
                    // Draw rough circular shape
                    for (int8_t dx = -displaySize; dx <= displaySize; dx++)
                    {
                        for (int8_t dy = -displaySize; dy <= displaySize; dy++)
                        {
                            if (abs(dx) + abs(dy) <= displaySize)
                            {
                                uint8_t noise = random8(brightness);
                                Gfx(screenX + dx, screenY + dy) +=
                                    ColorFromPalette(palette, obj.hue + baseHue + noise / 8, noise);
                            }
                        }
                    }
                }
                break;

                case 2: // Debris/particles
                {
                    // Draw small cluster
                    CRGB color = ColorFromPalette(palette, obj.hue + baseHue + obj.z / 4, brightness);
                    Gfx(screenX, screenY) += color;
                    if (displaySize > 0)
                    {
                        Gfx(screenX + 1, screenY) += color;
                        Gfx(screenX, screenY + 1) += color;
                    }
                }
                break;
            }

            // Motion blur lines for fast objects
            if (warpSpeed > 10 || audioBoost > 3)
            {
                uint8_t lineLength = min(8, warpSpeed / 2 + audioBoost);
                for (uint8_t l = 1; l < lineLength; l++)
                {
                    uint8_t blurX = screenX + (l * (screenX - MATRIX_CENTER_X) / 8);
                    uint8_t blurY = screenY + (l * (screenY - MATRIX_CENTER_Y) / 8);
                    if (blurX < MATRIX_WIDTH && blurY < MATRIX_HEIGHT)
                    {
                        Gfx(blurX, blurY) += ColorFromPalette(palette, obj.hue + baseHue, brightness / (l + 1));
                    }
                }
            }
        }

        // Add nebula/space dust effect based on audio
        for (uint8_t i = 0; i < 8; i++)
        {
            uint8_t x = beatsin8(10 + i * 3, 0, MATRIX_WIDTH - 1);
            uint8_t y = beatcos8(7 + i * 2, 0, MATRIX_HEIGHT - 1);
            uint8_t size = Audio.heights8[i * 8] >> 5;

            if (size > 0)
            {
                for (int8_t dx = -size; dx <= size; dx++)
                {
                    for (int8_t dy = -size; dy <= size; dy++)
                    {
                        uint8_t dist = abs(dx) + abs(dy);
                        if (dist <= size)
                        {
                            Gfx(x + dx, y + dy) += CHSV(baseHue + i * 32, 255 - dist * 30, 100 - dist * 20);
                        }
                    }
                }
            }
        }

        // Flash effect on beat
        if (beatFlash > 200)
        {
            Gfx.dim(255 - (beatFlash >> 2));
        }
    }
};
