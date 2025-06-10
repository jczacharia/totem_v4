#pragma once

class AudioRainMatrixPattern final : public Pattern
{
    // Rain drop tracking
    uint8_t columnY[MATRIX_WIDTH];       // Current Y position of each column's head
    uint8_t columnSpeed[MATRIX_WIDTH];   // Speed of each column (frames to skip)
    uint8_t columnCounter[MATRIX_WIDTH]; // Frame counter for each column
    uint8_t columnLength[MATRIX_WIDTH];  // Length of each rain trail
    uint8_t columnHue[MATRIX_WIDTH];     // Color of each column
    bool columnActive[MATRIX_WIDTH];     // Whether column is currently raining

    // Glitch effect variables
    uint8_t glitchIntensity = 0;
    uint8_t glitchLines[8]; // Y positions of glitch lines
    uint8_t glitchHues[8];  // Colors of glitch lines
    uint8_t glitchCounter = 0;

    // Pattern parameters
    uint8_t baseSpeed = 3;    // Base fall speed (higher = slower)
    uint8_t spawnChance = 8;  // Chance of new column spawning
    uint8_t fadeAmount = 240; // Trail fading amount
    uint8_t minTrailLength = 3;
    uint8_t maxTrailLength = 12;

    // Audio responsiveness
    uint8_t audioSensitivity = 6;
    uint8_t lastEnergyLevel = 0;

    void randomize()
    {
        // Randomize initial parameters
        baseSpeed = random8(2, 6);
        spawnChance = random8(5, 15);
        fadeAmount = random8(220, 250);
        minTrailLength = random8(2, 5);
        maxTrailLength = random8(8, 16);
        audioSensitivity = random8(4, 8);
    }

  public:
    static constexpr auto ID = "Audio Rain Matrix";

    AudioRainMatrixPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Initialize all columns as inactive
        for (uint8_t i = 0; i < MATRIX_WIDTH; i++)
        {
            columnActive[i] = false;
            columnCounter[i] = 0;
        }

        randomize();
        palette = randomPalette();
        glitchIntensity = 0;
        glitchCounter = 0;
    }

    void render() override
    {
        // Fade the existing display
        Gfx.dim(fadeAmount);

        if (Audio.isBeat && Audio.totalBeats % 4 == 0)
        {
            randomize();
        }

        // Audio-reactive spawning
        uint8_t currentEnergy = Audio.energy8 >> 4; // Scale down to 0-15
        uint8_t energyDiff = (currentEnergy > lastEnergyLevel) ? currentEnergy - lastEnergyLevel : 0;
        lastEnergyLevel = currentEnergy;

        // Update each column
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            // Try to spawn new column if inactive
            if (!columnActive[x])
            {
                uint8_t audioBoost = Audio.heights8[x] >> audioSensitivity;
                uint8_t spawnProbability = spawnChance + audioBoost + energyDiff;

                if (random8(100) < spawnProbability)
                {
                    // Initialize new column
                    columnY[x] = 0;
                    columnSpeed[x] = random8(1, baseSpeed + 3);
                    columnLength[x] = random8(minTrailLength, maxTrailLength + 1);
                    columnHue[x] = random8();
                    columnActive[x] = true;
                    columnCounter[x] = 0;
                }
                continue;
            }

            // Update active column
            columnCounter[x]++;

            // Check if it's time to advance this column
            uint8_t audioSpeedBoost = Audio.heights8[x] >> 6; // 0-3 speed boost
            uint8_t effectiveSpeed = (columnSpeed[x] > audioSpeedBoost) ? columnSpeed[x] - audioSpeedBoost : 1;

            if (columnCounter[x] >= effectiveSpeed)
            {
                columnCounter[x] = 0;
                columnY[x]++;

                // Draw the head (brightest)
                if (columnY[x] < MATRIX_HEIGHT)
                {
                    CRGB headColor = ColorFromPalette(palette, columnHue[x], 255);
                    Gfx.drawPixel(x, columnY[x], headColor);
                }

                // Draw the trail
                for (uint8_t i = 1; i < columnLength[x]; i++)
                {
                    int16_t trailY = columnY[x] - i;
                    if (trailY >= 0 && trailY < MATRIX_HEIGHT)
                    {
                        uint8_t brightness = 255 - (i * 255 / columnLength[x]);
                        brightness = brightness >> 1; // Make trail dimmer
                        CRGB trailColor = ColorFromPalette(palette, columnHue[x] + (i << 2), brightness);
                        Gfx.drawPixel(x, trailY, trailColor);
                    }
                }

                // Check if column has fallen off screen
                if (columnY[x] >= MATRIX_HEIGHT + columnLength[x])
                {
                    columnActive[x] = false;
                }
            }
        }

        // Draw glitch effects
        if (glitchCounter > 0)
        {
            glitchCounter--;
            glitchIntensity = (glitchIntensity * 220) >> 8; // Fade glitch

            for (uint8_t i = 0; i < 8; i++)
            {
                uint8_t glitchY = glitchLines[i];
                if (glitchY < MATRIX_HEIGHT)
                {
                    // Draw horizontal glitch line
                    for (uint8_t x = 0; x < MATRIX_WIDTH; x += 2)
                    {
                        if (random8(3) == 0) // Sparse glitch
                        {
                            CRGB glitchColor = CHSV(glitchHues[i], 255, glitchIntensity);
                            Gfx.drawPixel(x, glitchY, glitchColor);
                        }
                    }
                }
            }

            // Random pixel corruption during glitch
            if (glitchIntensity > 100)
            {
                for (uint8_t i = 0; i < 12; i++)
                {
                    uint8_t glitchX = random8(MATRIX_WIDTH);
                    uint8_t glitchY = random8(MATRIX_HEIGHT);
                    CRGB corruptColor = CHSV(random8(), 255, glitchIntensity);
                    Gfx.drawPixel(glitchX, glitchY, corruptColor);
                }
            }
        }

        // Apply kaleidoscope effects occasionally
        if (Audio.isBeat && random8(3) == 0)
        {
            Gfx.randomKaleidoscope(random8(1, KALEIDOSCOPE_COUNT + 1));
        }
    }
};
