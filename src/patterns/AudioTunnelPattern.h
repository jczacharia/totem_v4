#pragma once

class AudioTunnelPattern final : public Pattern
{
    struct TunnelRing
    {
        uint8_t depth;      // Z-depth (0 = far, 255 = close)
        uint8_t size;       // Ring size
        uint8_t hue;        // Color
        uint8_t brightness;
        bool active;
    };

    static constexpr uint8_t MAX_RINGS = 16;
    TunnelRing rings[MAX_RINGS];
    
    uint8_t tunnelSpeed = 3;
    uint8_t ringSpacing = 16;
    uint8_t colorOffset = 0;
    uint8_t rotationAngle = 0;
    uint8_t rotationSpeed = 2;
    uint8_t frameCounter = 0;
    bool reverseDirection = false;
    uint8_t tunnelShape = 0; // 0=rectangles, 1=circles, 2=diamonds
    uint8_t waveMode = 0;
    uint32_t lastBeatTime = 0;

    void randomize()
    {
        tunnelSpeed = random8(2, 6);
        ringSpacing = random8(12, 24);
        colorOffset = random8();
        rotationSpeed = random8(1, 4);
        reverseDirection = random8(2);
        tunnelShape = random8(3);
        waveMode = random8(3); // 0=none, 1=sine wave, 2=pulse
    }

public:
    static constexpr auto ID = "Audio Tunnel";

    AudioTunnelPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        // Initialize all rings as inactive
        for (uint8_t i = 0; i < MAX_RINGS; i++)
        {
            rings[i].active = false;
        }
        

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

        // Clear or fade background
        Gfx.dim(200);

        // Update rotation
        rotationAngle += rotationSpeed + (Audio.energy8 >> 6);

        // Update existing rings
        for (uint8_t i = 0; i < MAX_RINGS; i++)
        {
            if (!rings[i].active) continue;

            // Move ring based on audio energy and direction
            uint8_t speed = tunnelSpeed + (Audio.energy8 >> 5);
            
            if (reverseDirection)
            {
                // Rings move away
                rings[i].depth -= speed;
                if (rings[i].depth < 8)
                {
                    rings[i].active = false;
                    continue;
                }
            }
            else
            {
                // Rings move toward viewer
                rings[i].depth += speed;
                if (rings[i].depth > 250)
                {
                    rings[i].active = false;
                    continue;
                }
            }

            // Apply wave effects
            uint8_t waveOffset = 0;
            if (waveMode == 1) // Sine wave
            {
                waveOffset = sin8(frameCounter * 4 + rings[i].depth) >> 4;
            }
            else if (waveMode == 2) // Pulse
            {
                uint8_t pulse = sin8(frameCounter * 8 + rings[i].depth * 2);
                if (pulse > 200)
                {
                    waveOffset = 8;
                }
            }

            // Calculate ring size based on depth (perspective)
            uint8_t ringSize = (rings[i].size * rings[i].depth) >> 6;
            ringSize += waveOffset;
            
            if (ringSize > 31) ringSize = 31; // Max size for 64x64 matrix

            // Calculate brightness based on depth and audio
            uint8_t brightness = rings[i].depth;
            if (Audio.isBeat && millis() - lastBeatTime < 150)
            {
                brightness = qadd8(brightness, Audio.energy8 >> 2);
            }

            // Calculate color
            uint8_t hue = rings[i].hue + colorOffset + (rotationAngle >> 2);
            CRGB color = ColorFromPalette(palette, hue, brightness);

            // Draw the ring based on shape
            drawTunnelRing(ringSize, color, waveOffset);
        }

        // Spawn new rings based on audio
        frameCounter++;
        uint8_t spawnRate = 8 + (Audio.energy8 >> 5);
        
        if (frameCounter % (24 - (spawnRate >> 2)) == 0)
        {
            // Find inactive ring
            for (uint8_t i = 0; i < MAX_RINGS; i++)
            {
                if (!rings[i].active)
                {
                    rings[i].active = true;
                    rings[i].depth = reverseDirection ? 250 : 8;
                    rings[i].size = random8(8, 24);
                    rings[i].hue = colorOffset + random8(128);
                    rings[i].brightness = 255;
                    break;
                }
            }
        }

        // Color cycling
        colorOffset += 1;

        // Add special effects on strong beats
        if (Audio.isBeat && Audio.energy8 > 180)
        {
            // Flash effect - draw bright center
            uint8_t flashSize = 4 + (Audio.energy8 >> 5);
            CRGB flashColor = ColorFromPalette(palette, colorOffset + 64, 255);
            
            for (int8_t dx = -flashSize; dx <= flashSize; dx++)
            {
                for (int8_t dy = -flashSize; dy <= flashSize; dy++)
                {
                    uint8_t distance = abs(dx) + abs(dy);
                    if (distance <= flashSize)
                    {
                        uint8_t fade = 255 - (distance * 255 / flashSize);
                        int16_t x = MATRIX_CENTER_X + dx;
                        int16_t y = MATRIX_CENTER_Y + dy;
                        if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
                        {
                            Gfx(x, y) += flashColor.nscale8(fade);
                        }
                    }
                }
            }
        }
    }

private:
    void drawTunnelRing(uint8_t size, CRGB color, uint8_t waveOffset)
    {
        if (size == 0) return;
        
        // Apply rotation for more dynamic effect
        uint8_t rotatedAngle = rotationAngle + waveOffset;
        
        switch (tunnelShape)
        {
            case 0: // Rectangles
                drawRotatedRectangle(size, color, rotatedAngle);
                break;
            case 1: // Circles
                drawCircle(size, color);
                break;
            case 2: // Diamonds
                drawDiamond(size, color, rotatedAngle);
                break;
        }
    }
    
    void drawRotatedRectangle(uint8_t size, CRGB color, uint8_t angle)
    {
        // Simple rectangle with optional rotation effect
        uint8_t variation = (sin8(angle) >> 5);
        
        for (int8_t i = -size; i <= size; i++)
        {
            // Top and bottom edges
            int16_t x1 = MATRIX_CENTER_X + i;
            int16_t y1 = MATRIX_CENTER_Y - size + variation;
            int16_t y2 = MATRIX_CENTER_Y + size - variation;
            
            if (x1 >= 0 && x1 < MATRIX_WIDTH)
            {
                if (y1 >= 0 && y1 < MATRIX_HEIGHT) Gfx(x1, y1) += color;
                if (y2 >= 0 && y2 < MATRIX_HEIGHT) Gfx(x1, y2) += color;
            }
            
            // Left and right edges
            int16_t x2 = MATRIX_CENTER_X - size + variation;
            int16_t x3 = MATRIX_CENTER_X + size - variation;
            int16_t y3 = MATRIX_CENTER_Y + i;
            
            if (y3 >= 0 && y3 < MATRIX_HEIGHT)
            {
                if (x2 >= 0 && x2 < MATRIX_WIDTH) Gfx(x2, y3) += color;
                if (x3 >= 0 && x3 < MATRIX_WIDTH) Gfx(x3, y3) += color;
            }
        }
    }
    
    void drawCircle(uint8_t radius, CRGB color)
    {
        // Bresenham circle algorithm
        int16_t x = 0;
        int16_t y = radius;
        int16_t d = 1 - radius;
        
        while (x <= y)
        {
            // Draw 8 octants
            drawCirclePixel(MATRIX_CENTER_X + x, MATRIX_CENTER_Y + y, color);
            drawCirclePixel(MATRIX_CENTER_X - x, MATRIX_CENTER_Y + y, color);
            drawCirclePixel(MATRIX_CENTER_X + x, MATRIX_CENTER_Y - y, color);
            drawCirclePixel(MATRIX_CENTER_X - x, MATRIX_CENTER_Y - y, color);
            drawCirclePixel(MATRIX_CENTER_X + y, MATRIX_CENTER_Y + x, color);
            drawCirclePixel(MATRIX_CENTER_X - y, MATRIX_CENTER_Y + x, color);
            drawCirclePixel(MATRIX_CENTER_X + y, MATRIX_CENTER_Y - x, color);
            drawCirclePixel(MATRIX_CENTER_X - y, MATRIX_CENTER_Y - x, color);
            
            if (d < 0)
            {
                d += 2 * x + 3;
            }
            else
            {
                d += 2 * (x - y) + 5;
                y--;
            }
            x++;
        }
    }
    
    void drawCirclePixel(int16_t x, int16_t y, CRGB color)
    {
        if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
        {
            Gfx(x, y) += color;
        }
    }
    
    void drawDiamond(uint8_t size, CRGB color, uint8_t angle)
    {
        uint8_t variation = (cos8(angle) >> 5);
        
        // Draw diamond as four lines
        for (uint8_t i = 0; i <= size; i++)
        {
            // Top-left to top-right
            int16_t x1 = MATRIX_CENTER_X - size + i;
            int16_t y1 = MATRIX_CENTER_Y - i + variation;
            if (x1 >= 0 && x1 < MATRIX_WIDTH && y1 >= 0 && y1 < MATRIX_HEIGHT)
                Gfx(x1, y1) += color;
                
            // Top-right to bottom-right
            int16_t x2 = MATRIX_CENTER_X + i;
            int16_t y2 = MATRIX_CENTER_Y - size + i - variation;
            if (x2 >= 0 && x2 < MATRIX_WIDTH && y2 >= 0 && y2 < MATRIX_HEIGHT)
                Gfx(x2, y2) += color;
                
            // Bottom-right to bottom-left
            int16_t x3 = MATRIX_CENTER_X + size - i;
            int16_t y3 = MATRIX_CENTER_Y + i + variation;
            if (x3 >= 0 && x3 < MATRIX_WIDTH && y3 >= 0 && y3 < MATRIX_HEIGHT)
                Gfx(x3, y3) += color;
                
            // Bottom-left to top-left
            int16_t x4 = MATRIX_CENTER_X - i;
            int16_t y4 = MATRIX_CENTER_Y + size - i - variation;
            if (x4 >= 0 && x4 < MATRIX_WIDTH && y4 >= 0 && y4 < MATRIX_HEIGHT)
                Gfx(x4, y4) += color;
        }
    }
}; 