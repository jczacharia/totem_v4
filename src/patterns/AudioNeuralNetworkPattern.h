#pragma once

#include "Pattern.h"

class AudioNeuralNetworkPattern final : public Pattern
{
    static constexpr uint8_t MAX_NEURONS = 16;
    static constexpr uint8_t MAX_SYNAPSES = 24;

    struct Neuron
    {
        int16_t x, y, z;  // 3D position
        uint8_t activity; // Current activation level
        uint8_t hue;      // Color
        uint8_t size;     // Size multiplier
        bool firing;      // Currently firing
    };

    struct Synapse
    {
        uint8_t fromNeuron; // Source neuron index
        uint8_t toNeuron;   // Target neuron index
        uint8_t signal;     // Signal strength (0-255, 0=no signal)
        uint8_t progress;   // Signal travel progress (0-255)
        uint8_t hue;        // Signal color
        bool active;
    };

    Neuron neurons[MAX_NEURONS];
    Synapse synapses[MAX_SYNAPSES];
    uint8_t rotationX = 0;
    uint8_t rotationY = 0;
    uint8_t colorOffset = 0;
    uint8_t networkDepth = 100;
    uint8_t activeNeurons = MAX_NEURONS;
    uint8_t activeSynapses = MAX_SYNAPSES;

  public:
    static constexpr auto ID = "Neural Network";

    AudioNeuralNetworkPattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        networkDepth = random8(80, 120);
        activeNeurons = random8(8, MAX_NEURONS + 1);
        activeSynapses = random8(12, MAX_SYNAPSES + 1);

        // Setup neurons in 3D space
        for (uint8_t i = 0; i < MAX_NEURONS; i++)
        {
            neurons[i].x = random16(-networkDepth, networkDepth);
            neurons[i].y = random16(-networkDepth, networkDepth);
            neurons[i].z = random16(-networkDepth, networkDepth);
            neurons[i].activity = random8();
            neurons[i].hue = random8();
            neurons[i].size = random8(2, 5);
            neurons[i].firing = false;
        }

        // Setup synapses (connections between neurons)
        for (uint8_t i = 0; i < MAX_SYNAPSES; i++)
        {
            synapses[i].fromNeuron = random8(activeNeurons);
            synapses[i].toNeuron = random8(activeNeurons);
            synapses[i].signal = 0;
            synapses[i].progress = 0;
            synapses[i].hue = random8();
            synapses[i].active = (i < activeSynapses);
        }

        palette = randomPalette();
        kaleidoscope = random8(2);
        kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
    }

    void render() override
    {
        // Beat effects - trigger neural firing
        if (Audio.isBeat)
        {
            if (Audio.totalBeats % 4 == 0)
            {
                kaleidoscope = random8(2);
                kaleidoscopeMode = random8(1, KALEIDOSCOPE_COUNT + 1);
            }
            for (uint8_t i = 0; i < activeNeurons; i++)
            {
                if (random8() < 80)
                {
                    neurons[i].firing = true;
                    neurons[i].activity = 255;

                    // Trigger synapses from this neuron
                    for (uint8_t j = 0; j < activeSynapses; j++)
                    {
                        if (synapses[j].fromNeuron == i && synapses[j].signal == 0)
                        {
                            synapses[j].signal = 200 + (Audio.energy8 >> 2);
                            synapses[j].progress = 0;
                            synapses[j].hue = neurons[i].hue + random8(64);
                        }
                    }
                }
            }
        }

        // Fade background
        Gfx.dim(220);

        // Update rotations
        rotationX += 1 + (Audio.energy8 >> 8);
        rotationY += 2 + (Audio.energy8 >> 7);
        colorOffset += 3;

        // Update neuron activities
        for (uint8_t i = 0; i < activeNeurons; i++)
        {
            if (neurons[i].firing)
            {
                neurons[i].activity = max(neurons[i].activity - 10, 0);
                if (neurons[i].activity < 50)
                {
                    neurons[i].firing = false;
                }
            }
            else
            {
                // Gradual decay
                neurons[i].activity = max<uint8_t>(neurons[i].activity - 2, random8(30));
            }
        }

        // Update and draw synapses
        for (uint8_t i = 0; i < activeSynapses; i++)
        {
            if (!synapses[i].active)
                continue;

            Synapse &syn = synapses[i];

            if (syn.signal > 0)
            {
                syn.progress += 8 + (Audio.energy8 >> 6);

                if (syn.progress >= 255)
                {
                    // Signal reached target neuron
                    if (syn.toNeuron < activeNeurons)
                    {
                        neurons[syn.toNeuron].activity = min(neurons[syn.toNeuron].activity + syn.signal, 255);
                        neurons[syn.toNeuron].firing = true;
                    }
                    syn.signal = 0;
                    syn.progress = 0;
                }
                else
                {
                    // Draw signal traveling along synapse
                    Neuron &from = neurons[syn.fromNeuron];
                    Neuron &to = neurons[syn.toNeuron];

                    // Interpolate signal position
                    int16_t signalX = from.x + ((to.x - from.x) * syn.progress >> 8);
                    int16_t signalY = from.y + ((to.y - from.y) * syn.progress >> 8);
                    int16_t signalZ = from.z + ((to.z - from.z) * syn.progress >> 8);

                    // Apply rotations
                    int16_t rotX = signalX;
                    int16_t rotY = signalY;
                    int16_t rotZ = signalZ;

                    // Rotate around X axis
                    int16_t tempY = rotY;
                    rotY = (rotY * (cos8(rotationX) - 128) - rotZ * (sin8(rotationX) - 128)) >> 7;
                    rotZ = (tempY * (sin8(rotationX) - 128) + rotZ * (cos8(rotationX) - 128)) >> 7;

                    // Rotate around Y axis
                    int16_t tempX = rotX;
                    rotX = (rotX * (cos8(rotationY) - 128) + rotZ * (sin8(rotationY) - 128)) >> 7;
                    rotZ = (-tempX * (sin8(rotationY) - 128) + rotZ * (cos8(rotationY) - 128)) >> 7;

                    // Project to screen
                    int16_t screenX = MATRIX_CENTER_X + (rotX * 120) / (rotZ + 200);
                    int16_t screenY = MATRIX_CENTER_Y + (rotY * 120) / (rotZ + 200);

                    if (screenX >= 0 && screenX < MATRIX_WIDTH && screenY >= 0 && screenY < MATRIX_HEIGHT)
                    {
                        uint8_t brightness = syn.signal;
                        Gfx(screenX, screenY) += ColorFromPalette(palette, syn.hue + colorOffset, brightness);
                    }
                }
            }
        }

        // Draw neurons
        for (uint8_t i = 0; i < activeNeurons; i++)
        {
            Neuron &neuron = neurons[i];

            // Apply 3D rotations
            int16_t x = neuron.x;
            int16_t y = neuron.y;
            int16_t z = neuron.z;

            // Rotate around X axis
            int16_t tempY = y;
            y = (y * (cos8(rotationX) - 128) - z * (sin8(rotationX) - 128)) >> 7;
            z = (tempY * (sin8(rotationX) - 128) + z * (cos8(rotationX) - 128)) >> 7;

            // Rotate around Y axis
            int16_t tempX = x;
            x = (x * (cos8(rotationY) - 128) + z * (sin8(rotationY) - 128)) >> 7;
            z = (-tempX * (sin8(rotationY) - 128) + z * (cos8(rotationY) - 128)) >> 7;

            // Perspective projection
            int16_t screenX = MATRIX_CENTER_X + (x * 120) / (z + 200);
            int16_t screenY = MATRIX_CENTER_Y + (y * 120) / (z + 200);

            if (screenX >= 0 && screenX < MATRIX_WIDTH && screenY >= 0 && screenY < MATRIX_HEIGHT)
            {
                // Draw neuron body
                uint8_t brightness = neuron.activity;
                uint8_t hue = neuron.hue + colorOffset;

                // Core
                Gfx(screenX, screenY) += ColorFromPalette(palette, hue, brightness);

                // Dendrites/axons (glow around neuron)
                uint8_t glowSize = neuron.size + (neuron.firing ? 2 : 0);
                for (int8_t dx = -glowSize; dx <= glowSize; dx++)
                {
                    for (int8_t dy = -glowSize; dy <= glowSize; dy++)
                    {
                        if (dx == 0 && dy == 0)
                            continue;

                        uint8_t distance = abs(dx) + abs(dy);
                        if (distance <= glowSize)
                        {
                            int16_t glowX = screenX + dx;
                            int16_t glowY = screenY + dy;

                            if (glowX >= 0 && glowX < MATRIX_WIDTH && glowY >= 0 && glowY < MATRIX_HEIGHT)
                            {
                                uint8_t glowBrightness = brightness * (glowSize - distance) / (glowSize + 1);
                                glowBrightness >>= 2;
                                Gfx(glowX, glowY) += ColorFromPalette(palette, hue + 32, glowBrightness);
                            }
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
