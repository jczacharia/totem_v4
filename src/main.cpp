#include <Arduino.h>
#include <GFX_Layer.hpp>
#include <GFX_Lite.h>
#include <atomic>

#include "Matrix.h"
#include "Microphone.h"
#include "Registry.h"

#include "patterns/AudioSpectrumPattern.h"
// #include "patterns/Audio8x8SquaresPattern.h"
// #include "patterns/Audio3dGridPattern.h"
// #include "patterns/Audio2dGridPattern.h"
// #include "patterns/Audio2dWavesPattern.h"
// #include "patterns/AudioClassicSpectrum128Pattern.h"
// #include "patterns/AudioCubesPattern.h"
// #include "patterns/AudioDotsSinglePattern.h"
#include "patterns/AudioTrianglesPattern.h"
#include "patterns/AuroraDropPattern.h"
// #include "patterns/DNAHelixPattern.h"
// #include "patterns/JuliaFractalPattern.h"
#include "patterns/MandalaPattern.h"
// #include "patterns/MorphingShapesPattern.h"
// #include "patterns/NoopPattern.h"
// #include "patterns/PhyllotaxisFractalPattern.h"
// #include "patterns/RotatingSpectrumPattern.h"
// #include "patterns/Spectrum2Pattern.h"
// #include "patterns/SpectrumCirclePattern.h"

#include "playlists/MusicPlaylist.h"

static Microphone mic;
static std::atomic<uint8_t> globalBrightness{255};
static std::unique_ptr<MatrixPanel_I2S_DMA> dmaDisplay;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
    }

    HUB75_I2S_CFG cfg;
    cfg.gpio.r1 = 27;
    cfg.gpio.g1 = 26;
    cfg.gpio.b1 = 21;
    cfg.gpio.r2 = 12;
    cfg.gpio.g2 = 25;
    cfg.gpio.b2 = 19;
    cfg.gpio.a = 22;
    cfg.gpio.b = 18;
    cfg.gpio.c = 33;
    cfg.gpio.d = 13;
    cfg.gpio.e = 5;
    cfg.gpio.lat = 2;
    cfg.gpio.oe = 4;
    cfg.gpio.clk = 0;
    cfg.driver = HUB75_I2S_CFG::FM6126A;

    dmaDisplay = std::make_unique<MatrixPanel_I2S_DMA>(cfg);
    if (!dmaDisplay->begin())
    {
        Serial.println("Failed to start DMA Driver");
        ESP_INFINITE_LOOP();
    }
    Serial.println("Started DMA Driver");

    mic.start();
    delay(100);
    mic.getContext(Pattern::audio);
    random16_set_seed(UINT16_MAX * Pattern::audio.energy8 / 255);

    // Registry::add<Audio8x8SquaresPattern>();
    // Registry::add<Audio3dGridPattern>(leds, noise, audio);
    // Registry::add<Audio2dGridPattern>(leds, noise, audio);
    // Registry::add<AudioClassicSpectrum128Pattern>(leds, noise, audio);
    // Registry::add<AudioDotsSinglePattern>(leds, noise, audio);
    // Registry::add<Audio2dWavesPattern>(leds, noise, audio);
    // Registry::add<NoopPattern>(leds, noise, audio);
    Registry::add<MandalaPattern>();
    Registry::add<AudioSpectrumPattern>();
    Registry::add<AudioTrianglesPattern>();
    Registry::add<AuroraDropPattern>();
    // Registry::add<JuliaFractalPattern>(leds, noise, audio);
    // Registry::add<RotatingSpectrumPattern>(leds, noise, audio);
    // Registry::add<PhyllotaxisFractalPattern>(leds, noise, audio);
    // Registry::add<DNAHelixPattern>(leds, noise, audio);
    // Registry::add<SpectrumCirclePattern>(leds, noise, audio);
    // Registry::add<Spectrum2Pattern>(leds, noise, audio);
    // Registry::add<MorphingShapesPattern>(leds, noise, audio);
    // Registry::add<AudioCubesPattern>(leds, noise, audio);

    Registry::add<MusicPlaylist>();
    Registry::get(MusicPlaylist::ID)->start();
}

static uint32_t ms = 0;
static uint32_t fps = 0;

void loop()
{
    Pattern::leds.clear();
    Pattern::bkgLeds.clear();
    mic.getContext(Pattern::audio);
    random16_set_seed(UINT16_MAX * Pattern::audio.energy8 / 255);
    Registry::get(MusicPlaylist::ID)->render();
    dmaDisplay->setBrightness8(globalBrightness.load());

    for (size_t y = 0; y < dmaDisplay->height(); ++y)
    {
        for (size_t x = 0; x < dmaDisplay->width(); ++x)
        {
            const CRGB &led = Pattern::leds(x, y) + Pattern::bkgLeds(x, y);
            dmaDisplay->drawPixelRGB888(x, y, led.r, led.g, led.b);
        }
    }

    fps++;
    if (millis() - ms > 1000)
    {
        Serial.printf("FPS: %d\n", fps);
        ms = millis();
        fps = 0;
    }
}
