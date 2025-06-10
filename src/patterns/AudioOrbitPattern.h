#pragma once

class AudioOrbitPattern final : public Pattern
{
    struct GravityWell
    {
        int16_t x, y;     // Position
        uint8_t strength; // Gravitational strength
        uint8_t hue;      // Color
        bool active;
    };

    struct OrbitingObject
    {
        uint8_t angle;  // Current angle (0-255)
        uint8_t radius; // Orbital radius
        uint8_t speed;  // Orbital speed
        uint8_t hue;    // Color
        uint8_t brightness;
        uint8_t wellIndex;   // Which gravity well it orbits
        uint8_t trailLength; // Trail effect
        bool active;
    };

    static constexpr uint8_t MAX_WELLS = 3;
    static constexpr uint8_t MAX_OBJECTS = 32;

    GravityWell wells[MAX_WELLS];
    OrbitingObject objects[MAX_OBJECTS];

    uint8_t activeWells = 2;
    uint8_t colorOffset = 0;
    uint8_t frameCounter = 0;
    uint8_t baseSpeed = 2;
    bool showOrbitalPaths = false;
    bool connectObjects = false;
    uint8_t wellMovementSpeed = 1;
    uint32_t lastBeatTime = 0;

    void randomize()
    {
        activeWells = random8(2, MAX_WELLS + 1);
        baseSpeed = random8(1, 4);
        showOrbitalPaths = random8(2);
        connectObjects = random8(3) == 0; // 33% chance
        wellMovementSpeed = random8(1, 3);
        kaleidoscope = random8(2);
        kaleidoscopeMode = random8(1,KALEIDOSCOPE_COUNT+1);
    }

  public:
    static constexpr auto ID = "Audio Orbit";

    AudioOrbitPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Initialize gravity wells
        for (uint8_t i = 0; i < MAX_WELLS; i++)
        {
            wells[i].active = false;
        }

        // Initialize objects
        for (uint8_t i = 0; i < MAX_OBJECTS; i++)
        {
            objects[i].active = false;
        }

        randomize();
        palette = randomPalette();

        // Set up initial gravity wells
        for (uint8_t i = 0; i < activeWells; i++)
        {
            wells[i].active = true;
            wells[i].x = MATRIX_CENTER_X + random8(32) - 16;
            wells[i].y = MATRIX_CENTER_Y + random8(32) - 16;
            wells[i].strength = random8(8, 16);
            wells[i].hue = random8();
        }
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

            if (Audio.totalBeats % 8 == 0)
            {
                // Randomly activate/deactivate wells
                for (uint8_t i = 0; i < MAX_WELLS; i++)
                {
                    if (i < activeWells)
                    {
                        wells[i].active = true;
                    }
                    else
                    {
                        wells[i].active = random8(4) == 0; // 25% chance
                    }
                }
            }
        }

        // Fade background
        Gfx.dim(230);

        frameCounter++;

                // Update gravity well positions based on audio
        for (uint8_t i = 0; i < MAX_WELLS; i++)
        {
            if (!wells[i].active)
                continue;
            
            // Move wells in circular patterns influenced by audio
            uint8_t wellAngle = frameCounter * wellMovementSpeed + i * 85;
            uint8_t audioInfluence = Audio.energy8 >> 6; // Reduced influence
            
            // Keep movement radius smaller and more controlled
            uint8_t moveRadius = 6 + audioInfluence; // Smaller base radius
            
            // Use smaller movement range to keep wells more centered
            int8_t dx = ((sin8(wellAngle) - 128) * moveRadius) >> 8; // Divide by 256 instead of 128
            int8_t dy = ((cos8(wellAngle + 64) - 128) * moveRadius) >> 8;
            
            wells[i].x = MATRIX_CENTER_X + dx;
            wells[i].y = MATRIX_CENTER_Y + dy;
            
            // Keep wells closer to center with tighter bounds
            if (wells[i].x < 8)
                wells[i].x = 8;
            if (wells[i].x > MATRIX_WIDTH - 8)
                wells[i].x = MATRIX_WIDTH - 8;
            if (wells[i].y < 8)
                wells[i].y = 8;
            if (wells[i].y > MATRIX_HEIGHT - 8)
                wells[i].y = MATRIX_HEIGHT - 8;
        }

        // Update orbiting objects
        for (uint8_t i = 0; i < MAX_OBJECTS; i++)
        {
            if (!objects[i].active)
                continue;

            // Make sure the object's well is still active
            if (!wells[objects[i].wellIndex].active)
            {
                objects[i].active = false;
                continue;
            }

            // Update orbital angle based on speed and audio
            uint8_t speed = objects[i].speed + baseSpeed + (Audio.energy8 >> 6);
            objects[i].angle += speed;

            // Calculate orbital position
            GravityWell &well = wells[objects[i].wellIndex];
            int16_t dx = ((sin8(objects[i].angle) - 128) * objects[i].radius) >> 7;
            int16_t dy = ((cos8(objects[i].angle) - 128) * objects[i].radius) >> 7;

            int16_t objX = well.x + dx;
            int16_t objY = well.y + dy;

            // Calculate brightness based on audio and distance from center
            uint8_t brightness = objects[i].brightness;
            if (Audio.isBeat && millis() - lastBeatTime < 150)
            {
                brightness = qadd8(brightness, Audio.energy8 >> 2);
            }

            // Draw the object
            if (objX >= 0 && objX < MATRIX_WIDTH && objY >= 0 && objY < MATRIX_HEIGHT)
            {
                CRGB color = ColorFromPalette(palette, objects[i].hue + colorOffset, brightness);
                Gfx(objX, objY) += color;

                // Draw larger objects for closer orbits
                if (objects[i].radius < 12)
                {
                    if (objX > 0)
                        Gfx(objX - 1, objY) += color.nscale8(128);
                    if (objX < MATRIX_WIDTH - 1)
                        Gfx(objX + 1, objY) += color.nscale8(128);
                    if (objY > 0)
                        Gfx(objX, objY - 1) += color.nscale8(128);
                    if (objY < MATRIX_HEIGHT - 1)
                        Gfx(objX, objY + 1) += color.nscale8(128);
                }
            }
        }

        // Draw gravity wells
        for (uint8_t i = 0; i < MAX_WELLS; i++)
        {
            if (!wells[i].active)
                continue;

            uint8_t wellBrightness = 128 + (Audio.energy8 >> 3);
            CRGB wellColor = ColorFromPalette(palette, wells[i].hue + colorOffset, wellBrightness);

            // Draw well as a cross or plus sign
            if (wells[i].x >= 1 && wells[i].x < MATRIX_WIDTH - 1)
            {
                Gfx(wells[i].x - 1, wells[i].y) += wellColor;
                Gfx(wells[i].x + 1, wells[i].y) += wellColor;
            }
            if (wells[i].y >= 1 && wells[i].y < MATRIX_HEIGHT - 1)
            {
                Gfx(wells[i].x, wells[i].y - 1) += wellColor;
                Gfx(wells[i].x, wells[i].y + 1) += wellColor;
            }
            Gfx(wells[i].x, wells[i].y) += wellColor;
        }

        // Draw orbital paths
        if (showOrbitalPaths)
        {
            for (uint8_t i = 0; i < MAX_OBJECTS; i++)
            {
                if (!objects[i].active || !wells[objects[i].wellIndex].active)
                    continue;

                GravityWell &well = wells[objects[i].wellIndex];

                // Draw a few points around the orbital path
                for (uint8_t a = 0; a < 8; a++)
                {
                    uint8_t pathAngle = a * 32; // 8 points around circle
                    int16_t px = well.x + ((sin8(pathAngle) - 128) * objects[i].radius) >> 7;
                    int16_t py = well.y + ((cos8(pathAngle) - 128) * objects[i].radius) >> 7;

                    if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT)
                    {
                        Gfx(px, py) += ColorFromPalette(palette, objects[i].hue + colorOffset, 32);
                    }
                }
            }
        }

        // Connect objects with lines
        if (connectObjects)
        {
            for (uint8_t i = 0; i < MAX_OBJECTS - 1; i++)
            {
                if (!objects[i].active)
                    continue;

                for (uint8_t j = i + 1; j < MAX_OBJECTS; j++)
                {
                    if (!objects[j].active)
                        continue;

                    // Only connect objects from different wells
                    if (objects[i].wellIndex != objects[j].wellIndex)
                    {
                        GravityWell &well1 = wells[objects[i].wellIndex];
                        GravityWell &well2 = wells[objects[j].wellIndex];

                        int16_t x1 = well1.x + ((sin8(objects[i].angle) - 128) * objects[i].radius) >> 7;
                        int16_t y1 = well1.y + ((cos8(objects[i].angle) - 128) * objects[i].radius) >> 7;
                        int16_t x2 = well2.x + ((sin8(objects[j].angle) - 128) * objects[j].radius) >> 7;
                        int16_t y2 = well2.y + ((cos8(objects[j].angle) - 128) * objects[j].radius) >> 7;

                        Gfx.drawLine(x1, y1, x2, y2, ColorFromPalette(palette, colorOffset + 128, 64));
                    }
                }
            }
        }

        uint8_t spawnRate = 20 - min(Audio.energy8 >> 2, 19); // Ensure minimum of 1
        if (frameCounter % spawnRate == 0)
        {
            // Find inactive object
            for (uint8_t i = 0; i < MAX_OBJECTS; i++)
            {
                if (!objects[i].active)
                {
                    objects[i].active = true;
                    objects[i].wellIndex = random8(activeWells);

                    // Make sure the chosen well is active
                    while (!wells[objects[i].wellIndex].active && objects[i].wellIndex < MAX_WELLS)
                    {
                        objects[i].wellIndex = (objects[i].wellIndex + 1) % MAX_WELLS;
                    }

                    objects[i].angle = random8();
                    objects[i].radius = random8(6, 20);
                    objects[i].speed = random8(1, 4);
                    objects[i].hue = colorOffset + random8(64);
                    objects[i].brightness = 200 + random8(55);
                    objects[i].trailLength = random8(2, 6);
                    break;
                }
            }
        }

        // Color cycling
        colorOffset += 1;

        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
            Gfx.kaleidoscope1();
        }
    }
};
