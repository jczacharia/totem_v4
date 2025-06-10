#pragma once

#include "Pattern.h"
#include "Util.h"

class AudioBreathingRingsPattern final : public Pattern
{
    uint8_t ringPhase[4] = {0, 64, 128, 192}; // Different phases for 4 rings
    uint8_t colorRotation = 0;
    uint8_t breathingSpeed = 2;
    uint8_t maxRings = 4;
    
    // Ripple variables
    uint8_t ripplePhase[3] = {0, 0, 0}; // Up to 3 active ripples
    uint8_t rippleIntensity[3] = {0, 0, 0};
    uint8_t rippleAge[3] = {255, 255, 255}; // 255 = inactive
    uint8_t nextRippleSlot = 0;
    
    // Beat pulse variables
    uint8_t beatPulse = 0; // Current pulse strength
    uint8_t beatPulseDecay = 5; // How fast pulse fades

  public:
    static constexpr auto ID = "BreathingRings";

    AudioBreathingRingsPattern()
        : Pattern(ID)
    {
    }

        void start() override
    {
        breathingSpeed = random8(1, 4);
        maxRings = random8(2, 5);
        
        // Initialize ring phases with random offsets
        for (uint8_t i = 0; i < 4; i++)
        {
            ringPhase[i] = i * (256 / maxRings) + random8(32);
        }
        
        // Initialize ripples as inactive
        for (uint8_t i = 0; i < 3; i++)
        {
            rippleAge[i] = 255;
            rippleIntensity[i] = 0;
        }
        
        // Initialize beat pulse
        beatPulse = 0;
        beatPulseDecay = random8(3, 8);
        
        palette = randomPalette();
        kaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    }

        void render() override
    {
        // Beat effects - trigger ripples and pulse on beats
        if (Audio.isBeat)
        {
            // Start a new ripple on beat
            rippleAge[nextRippleSlot] = 0;
            ripplePhase[nextRippleSlot] = 0;
            rippleIntensity[nextRippleSlot] = Audio.energy8;
            nextRippleSlot = (nextRippleSlot + 1) % 3;
            
            // Trigger beat pulse based on audio energy
            beatPulse = Audio.energy8;
            
            if (Audio.totalBeats % 4 == 0)
            {
                kaleidoscope = random8(2);
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
                maxRings = random8(2, 5);
            }
        }
        
        // Update breathing phases and color rotation
        for (uint8_t i = 0; i < maxRings; i++)
        {
            ringPhase[i] += breathingSpeed + (Audio.energy8 >> 5);
        }
        colorRotation += 1;
        
        // Update beat pulse - fade out gradually
        if (beatPulse > beatPulseDecay)
        {
            beatPulse -= beatPulseDecay;
        }
        else
        {
            beatPulse = 0;
        }
        
        // Update ripples
        for (uint8_t i = 0; i < 3; i++)
        {
            if (rippleAge[i] < 255)
            {
                rippleAge[i] += 3; // Ripple speed
                ripplePhase[i] += 8; // Wave frequency
                if (rippleAge[i] > 200) // Fade out ripple
                {
                    rippleAge[i] = 255;
                    rippleIntensity[i] = 0;
                }
            }
        }

        // Draw breathing rings
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                                // Calculate distance from center
                int16_t dx = x - MATRIX_CENTER_X;
                int16_t dy = y - MATRIX_CENTER_Y;
                uint8_t distance = sqrt16(dx * dx + dy * dy);
                
                CRGB finalColor = CRGB::Black;
                
                // Calculate ripple effects
                uint8_t rippleEffect = 0;
                for (uint8_t r = 0; r < 3; r++)
                {
                    if (rippleAge[r] < 255)
                    {
                        // Calculate ripple wave at this distance
                        uint8_t rippleDistance = rippleAge[r];
                        if (distance >= rippleDistance - 4 && distance <= rippleDistance + 4)
                        {
                            // Create wave pattern
                            uint8_t waveIntensity = sin8(ripplePhase[r] + (distance << 3));
                            uint8_t fadeOut = 255 - rippleAge[r]; // Fade as ripple ages
                            uint8_t rippleStrength = scale8(waveIntensity, fadeOut);
                            rippleStrength = scale8(rippleStrength, rippleIntensity[r]);
                            rippleEffect = qadd8(rippleEffect, rippleStrength >> 1);
                        }
                    }
                }

                // Process each ring
                for (uint8_t ring = 0; ring < maxRings; ring++)
                {
                    // Calculate breathing radius for this ring
                    uint8_t baseRadius = 8 + (ring * 6);
                    uint8_t breathingOffset = (sin8(ringPhase[ring]) >> 3); // 0-32 range
                    
                    // Add beat pulse expansion
                    uint8_t pulseExpansion = (beatPulse >> 4); // 0-16 range
                    uint8_t currentRadius = baseRadius + breathingOffset + pulseExpansion;

                    // Create ring with soft edges
                    if (distance >= currentRadius - 3 && distance <= currentRadius + 3)
                    {
                        // Calculate ring intensity based on distance from ideal radius
                        uint8_t ringDistance =
                            (distance > currentRadius) ? (distance - currentRadius) : (currentRadius - distance);
                        uint8_t intensity = 255 - (ringDistance * 85); // Soft falloff

                                                // Audio-reactive intensity boost
                        intensity = qadd8(intensity, Audio.energy8 >> 2);
                        
                        // Add ripple effect to ring intensity
                        intensity = qadd8(intensity, rippleEffect);
                        
                        // Add beat pulse brightness boost
                        intensity = qadd8(intensity, beatPulse >> 1);
                        
                        // Ring color based on phase and audio
                        uint8_t hue = colorRotation + (ring * 64) + (ringPhase[ring] >> 3);
                        CRGB ringColor = ColorFromPalette(palette, hue, intensity);
                        
                        // Additive blending for overlapping rings
                        finalColor += ringColor;
                    }
                }
                
                // Add standalone ripple colors where no rings exist
                if (finalColor.r == 0 && finalColor.g == 0 && finalColor.b == 0 && rippleEffect > 32)
                {
                    uint8_t rippleHue = colorRotation + (distance << 2);
                    finalColor = ColorFromPalette(palette, rippleHue, rippleEffect);
                }
                
                Gfx(x, y) = finalColor;
            }
        }

        // Apply kaleidoscope effect occasionally
        if (kaleidoscope)
        {
            Gfx.randomKaleidoscope(kaleidoscopeMode);
        }
    }
};
