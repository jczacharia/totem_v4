#pragma once

class AudioStarfieldPattern final : public Pattern
{
    struct Star
    {
        int16_t x, y, z; // Fixed point coordinates (z is depth)
        uint8_t hue;
        uint8_t brightness;
        bool active;
    };

    static constexpr uint8_t MAX_STARS = 64;
    Star stars[MAX_STARS];

    uint8_t starCount = 0;
    uint8_t colorOffset = 0;
    uint8_t speedBase = 2;
    uint8_t spawnRate = 4;
    uint8_t frameCounter = 0;
    uint8_t warpMode = 0;
    uint32_t lastBeatTime = 0;

    void randomize()
    {
        colorOffset = random8();
        speedBase = random8(1, 4);
        spawnRate = random8(10, 20);
        warpMode = random8(3); // 0=normal, 1=spiral, 2=pulse
    }

  public:
    static constexpr auto ID = "Audio Starfield";

    AudioStarfieldPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Initialize all stars as inactive
        for (uint8_t i = 0; i < MAX_STARS; i++)
        {
            stars[i].active = false;
        }

        starCount = 0;
        palette = randomPalette();
    }

    void render() override
    {
        // Handle beat effects
        if (Audio.isBeat)
        {
            lastBeatTime = millis();
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        // Update existing stars
        for (uint8_t i = 0; i < MAX_STARS; i++)
        {
            if (!stars[i].active)
                continue;

            // Move star based on audio energy and direction
            uint8_t speed = speedBase + (Audio.energy8 >> 5);

            // Stars move outward
            stars[i].z -= speed;
            if (stars[i].z < 8)
            {
                stars[i].active = false;
                continue;
            }

            // Apply warp effects
            if (warpMode == 1) // Spiral
            {
                uint8_t angle = (frameCounter + i * 16) & 0xFF;
                stars[i].x += (sin8(angle) - 128) >> 6;
                stars[i].y += (cos8(angle) - 128) >> 6;
            }
            else if (warpMode == 2) // Pulse
            {
                uint8_t pulse = sin8(frameCounter * 4 + i * 32);
                if (pulse > 200)
                {
                    stars[i].z -= 2;
                }
            }

            // Calculate screen position (perspective projection)
            int16_t screenX = MATRIX_CENTER_X + (stars[i].x * 64) / (stars[i].z + 1);
            int16_t screenY = MATRIX_CENTER_Y + (stars[i].y * 64) / (stars[i].z + 1);

            // Calculate brightness based on depth and audio
            uint8_t brightness = 255 - stars[i].z;
            if (Audio.isBeat && millis() - lastBeatTime < 200)
            {
                brightness = qadd8(brightness, Audio.energy8 >> 1);
            }

            // Draw star if on screen
            if (screenX >= 0 && screenX < MATRIX_WIDTH && screenY >= 0 && screenY < MATRIX_HEIGHT)
            {
                CRGB color = ColorFromPalette(palette, stars[i].hue + colorOffset, brightness);

                // Draw star with size based on depth
                Gfx(screenX, screenY) += color;

                // Bigger stars for closer objects
                if (stars[i].z < 64)
                {
                    if (screenX > 0)
                        Gfx(screenX - 1, screenY) += color;
                    if (screenX < MATRIX_WIDTH - 1)
                        Gfx(screenX + 1, screenY) += color;
                    if (screenY > 0)
                        Gfx(screenX, screenY - 1) += color;
                    if (screenY < MATRIX_HEIGHT - 1)
                        Gfx(screenX, screenY + 1) += color;
                }
            }
        }

        // Spawn new stars based on audio
        frameCounter++;
        uint8_t actualSpawnRate = spawnRate + (Audio.energy8 >> 4);

        if (frameCounter % (16 - (actualSpawnRate >> 1)) == 0)
        {
            // Find inactive star
            for (uint8_t i = 0; i < MAX_STARS; i++)
            {
                if (!stars[i].active)
                {
                    stars[i].active = true;

                    // Start position (center with slight randomness)
                    stars[i].x = random8(16) - 8;
                    stars[i].y = random8(16) - 8;
                    stars[i].z = 200;

                    // Color based on audio frequency
                    stars[i].hue = colorOffset + random8(64);
                    stars[i].brightness = 255;

                    starCount++;
                    break;
                }
            }
        }

        // Color cycling
        colorOffset += 1;

        // // Add hyperspace effect on strong beats
        // if (Audio.isBeat && Audio.energy8 > 200)
        // {
        // Draw streaks for warp effect
        for (uint8_t i = 0; i < MAX_STARS; i++)
        {
            if (!stars[i].active)
                continue;

            int16_t screenX = MATRIX_CENTER_X + (stars[i].x * 64) / (stars[i].z + 1);
            int16_t screenY = MATRIX_CENTER_Y + (stars[i].y * 64) / (stars[i].z + 1);

            if (screenX >= 0 && screenX < MATRIX_WIDTH && screenY >= 0 && screenY < MATRIX_HEIGHT)
            {
                // Draw streak toward center
                int16_t dx = screenX - MATRIX_CENTER_X;
                int16_t dy = screenY - MATRIX_CENTER_Y;

                if (dx != 0 || dy != 0)
                {
                    int16_t streakX = screenX - (dx >> 2);
                    int16_t streakY = screenY - (dy >> 2);

                    if (streakX >= 0 && streakX < MATRIX_WIDTH && streakY >= 0 && streakY < MATRIX_HEIGHT)
                    {
                        Gfx.drawLine(
                            screenX,
                            screenY,
                            streakX,
                            streakY,
                            ColorFromPalette(palette, stars[i].hue + colorOffset, 128));
                    }
                }
            }
            // }
        }
    }
};
