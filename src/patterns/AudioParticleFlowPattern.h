#pragma once

#include "Pattern.h"

class AudioParticleFlowPattern final : public Pattern
{
    // Particle structure
    struct Particle
    {
        uint8_t x;
        uint8_t y;
        int8_t vx; // velocity x (-128 to 127)
        int8_t vy; // velocity y (-128 to 127)
        uint8_t life;
        uint8_t hue;
        bool active;
    };

    static constexpr uint8_t MAX_PARTICLES = 64;
    Particle particles[MAX_PARTICLES]{};

    // Flow field parameters
    uint8_t flowFieldScale = 16;
    uint16_t flowFieldOffset = 0;
    uint8_t flowFieldSpeed = 1;
    uint8_t gravityStrength = 0;
    uint8_t attractorX = MATRIX_CENTER_X;
    uint8_t attractorY = MATRIX_CENTER_Y;
    bool useAttractor = false;

    // Visual parameters
    uint8_t baseHue = 0;
    uint8_t hueSpeed = 1;
    uint8_t fadeAmount = 235;
    uint8_t particleSize = 1;
    bool drawTails = true;
    bool rainbowMode = false;

    // Audio response
    uint8_t audioSensitivity = 4;
    uint8_t spawnRate = 2;
    uint8_t maxSpeed = 4;

    // Pattern variations
    uint8_t spawnPattern = 0; // 0=edges, 1=center, 2=random, 3=bottom
    uint8_t flowType = 0;     // 0=circular, 1=turbulent, 2=directional
    bool reverseFlow = false;

    // Beat response
    uint32_t lastBeatTime = 0;
    uint8_t beatExplosionForce = 0;

    void randomize()
    {
        // Randomize parameters
        flowFieldScale = random8(10, 25);
        flowFieldSpeed = random8(1, 3);
        gravityStrength = random8(0, 3);
        useAttractor = random8(3) == 0; // 33% chance

        baseHue = random8();
        hueSpeed = random8(1, 3);
        fadeAmount = random8(220, 245);
        particleSize = random8(1, 3);
        drawTails = random8(3) > 0;    // 66% chance
        rainbowMode = random8(3) == 0; // 33% chance

        audioSensitivity = random8(3, 6);
        spawnRate = random8(1, 4);
        maxSpeed = random8(3, 6);

        spawnPattern = random8(4);
        flowType = random8(3);
        reverseFlow = random8(2);
    }

  public:
    static constexpr auto ID = "Audio Particle Flow";

    AudioParticleFlowPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Initialize particles
        for (uint8_t i = 0; i < MAX_PARTICLES; i++)
        {
            particles[i].active = false;
        }

        randomize();
        palette = randomPalette();

        // Start with some particles
        for (uint8_t s = 0; s < 8; s++)
        {
            for (uint8_t i = 0; i < MAX_PARTICLES; i++)
            {
                if (!particles[i].active)
                {
                    particles[i].active = true;
                    particles[i].life = 100 + random8(155);
                    particles[i].x = random8(MATRIX_WIDTH);
                    particles[i].y = random8(MATRIX_HEIGHT);
                    particles[i].vx = random8(3) - 1;
                    particles[i].vy = random8(3) - 1;
                    particles[i].hue = baseHue + random8(64);
                    break;
                }
            }
        }
    }

    void render() override
    {
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                randomize();
            }
        }

        // Fade effect
        if (drawTails)
        {
            Gfx.dim(fadeAmount);
        }
        else
        {
            Gfx.clear();
        }

        // Update flow field
        flowFieldOffset += flowFieldSpeed;

        // Update base hue
        baseHue += hueSpeed;

        // Beat detection
        if (Audio.isBeat)
        {
            lastBeatTime = millis();
            beatExplosionForce = 30;

            // Change attractor position on beat
            if (useAttractor && random8(4) == 0)
            {
                attractorX = random8(MATRIX_WIDTH);
                attractorY = random8(MATRIX_HEIGHT);
            }

            // Sometimes reverse flow
            if (random8(10) == 0)
            {
                reverseFlow = !reverseFlow;
            }
        }

        // Decay beat explosion
        if (beatExplosionForce > 0)
        {
            beatExplosionForce -= 2;
        }

        // Spawn new particles based on audio
        uint8_t particlesToSpawn = (Audio.energy8 >> 6) + 1;
        for (uint8_t s = 0; s < particlesToSpawn && s < spawnRate; s++)
        {
            if (random8(100) < 30 + (Audio.energy8 >> 2))
            {
                // Find inactive particle to spawn
                for (uint8_t i = 0; i < MAX_PARTICLES; i++)
                {
                    if (!particles[i].active)
                    {
                        particles[i].active = true;
                        particles[i].life = 100 + random8(155);

                        // Set spawn position based on pattern
                        switch (spawnPattern)
                        {
                            case 0: // Edges
                                if (random8(2))
                                {
                                    particles[i].x = random8(2) * (MATRIX_WIDTH - 1);
                                    particles[i].y = random8(MATRIX_HEIGHT);
                                }
                                else
                                {
                                    particles[i].x = random8(MATRIX_WIDTH);
                                    particles[i].y = random8(2) * (MATRIX_HEIGHT - 1);
                                }
                                break;

                            case 1: // Center
                                particles[i].x = MATRIX_CENTER_X + random8(16) - 8;
                                particles[i].y = MATRIX_CENTER_Y + random8(16) - 8;
                                break;

                            case 2: // Random
                                particles[i].x = random8(MATRIX_WIDTH);
                                particles[i].y = random8(MATRIX_HEIGHT);
                                break;

                            case 3: // Bottom (fountain)
                                particles[i].x = random8(MATRIX_WIDTH);
                                particles[i].y = MATRIX_HEIGHT - 1;
                                particles[i].vy = -random8(2, 5); // Upward velocity
                                break;
                        }

                        // Set initial velocity
                        if (spawnPattern != 3) // Not fountain mode
                        {
                            particles[i].vx = random8(3) - 1;
                            particles[i].vy = random8(3) - 1;
                        }
                        else
                        {
                            particles[i].vx = random8(3) - 1;
                        }

                        // Set color based on spawn position or audio
                        uint8_t audioIndex = particles[i].x * BINS / MATRIX_WIDTH;
                        particles[i].hue = baseHue + (Audio.heights8[audioIndex] >> 1);

                        break;
                    }
                }
            }
        }

        // Update and draw particles
        for (uint8_t i = 0; i < MAX_PARTICLES; i++)
        {
            if (!particles[i].active)
                continue;

            // Apply flow field forces
            uint8_t fieldX = particles[i].x >> 2;
            uint8_t fieldY = particles[i].y >> 2;
            uint8_t fieldIndex = (fieldX + fieldY * 16) & 0xFF;

            int8_t forceX = 0;
            int8_t forceY = 0;

            switch (flowType)
            {
                case 0: // Circular flow
                {
                    uint8_t angle = sin8(fieldIndex + flowFieldOffset) >> 1;
                    if (reverseFlow)
                        angle = 255 - angle;
                    forceX = (cos8(angle) - 128) >> 5;
                    forceY = (sin8(angle) - 128) >> 5;
                    break;
                }
                case 1: // Turbulent flow
                {
                    uint8_t noise1 = sin8(fieldX * flowFieldScale + flowFieldOffset);
                    uint8_t noise2 = cos8(fieldY * flowFieldScale + flowFieldOffset);
                    forceX = ((noise1 - 128) >> 5);
                    forceY = ((noise2 - 128) >> 5);
                    if (reverseFlow)
                    {
                        forceX = -forceX;
                        forceY = -forceY;
                    }
                    break;
                }
                case 2: // Directional flow with audio
                {
                    uint8_t audioIndex = particles[i].x * BINS / MATRIX_WIDTH;
                    uint8_t audioForce = Audio.heights8[audioIndex] >> audioSensitivity;
                    forceX = reverseFlow ? -1 : 1;
                    forceY = (audioForce - 16) >> 3;
                    break;
                }
            }

            // Apply attractor force
            if (useAttractor)
            {
                int8_t dx = attractorX - particles[i].x;
                int8_t dy = attractorY - particles[i].y;
                uint8_t distance = abs(dx) + abs(dy);
                if (distance > 0 && distance < 32)
                {
                    forceX += (dx >> 4);
                    forceY += (dy >> 4);
                }
            }

            // Apply gravity
            if (gravityStrength > 0)
            {
                forceY += gravityStrength;
            }

            // Apply beat explosion
            if (beatExplosionForce > 0)
            {
                int8_t dx = particles[i].x - MATRIX_CENTER_X;
                int8_t dy = particles[i].y - MATRIX_CENTER_Y;
                forceX += (dx * beatExplosionForce) >> 8;
                forceY += (dy * beatExplosionForce) >> 8;
            }

            // Update velocity with forces
            particles[i].vx = constrain(particles[i].vx + forceX, -maxSpeed, maxSpeed);
            particles[i].vy = constrain(particles[i].vy + forceY, -maxSpeed, maxSpeed);

            // Update position
            int16_t newX = particles[i].x + particles[i].vx;
            int16_t newY = particles[i].y + particles[i].vy;

            // Boundary behavior
            if (newX < 0 || newX >= MATRIX_WIDTH || newY < 0 || newY >= MATRIX_HEIGHT)
            {
                // Respawn particle
                particles[i].active = false;
                continue;
            }

            particles[i].x = newX;
            particles[i].y = newY;

            // Update life
            if (particles[i].life > 0)
            {
                particles[i].life--;
            }
            else
            {
                particles[i].active = false;
                continue;
            }

            // Calculate color
            uint8_t hue;
            if (rainbowMode)
            {
                hue = baseHue + (i << 3) + (particles[i].life >> 1);
            }
            else
            {
                hue = particles[i].hue + (particles[i].life >> 2);
            }

            // Brightness based on life and audio
            uint8_t brightness = scale8(particles[i].life << 1, 200 + (Audio.energy8 >> 2));

            // Draw particle
            for (uint8_t px = 0; px < particleSize; px++)
            {
                for (uint8_t py = 0; py < particleSize; py++)
                {
                    Gfx(particles[i].x + px, particles[i].y + py) += ColorFromPalette(palette, hue, brightness);
                }
            }

            // Draw motion blur trail
            if (drawTails && (abs(particles[i].vx) > 2 || abs(particles[i].vy) > 2))
            {
                int8_t trailX = particles[i].x - (particles[i].vx >> 1);
                int8_t trailY = particles[i].y - (particles[i].vy >> 1);
                if (trailX >= 0 && trailX < MATRIX_WIDTH && trailY >= 0 && trailY < MATRIX_HEIGHT)
                {
                    Gfx(trailX, trailY) += ColorFromPalette(palette, hue + 16, brightness >> 1);
                }
            }
        }

        // Draw audio spectrum as background glow
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            uint8_t height = Audio.heights8[x] >> 5;
            if (height > 0)
            {
                for (uint8_t y = MATRIX_HEIGHT - height; y < MATRIX_HEIGHT; y++)
                {
                    Gfx(x, y) += CHSV(baseHue + (x << 1), 255, height << 2);
                }
            }
        }

        // Apply kaleidoscope occasionally
        if (kaleidoscope && Audio.energy8 > 200)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
};
