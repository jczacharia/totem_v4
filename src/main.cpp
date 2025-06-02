#include <Arduino.h>
#include <GFX_Lite.h>
#include <GFX_Layer.hpp>
#include <atomic>

#include "Matrix.h"
#include "PatternRegistry.h"
#include "Microphone.h"

#include "patterns/TestPattern.h"
#include "patterns/MandalaPattern.h"
#include "patterns/TestPattern2.h"
#include "patterns/AudioSpectrumPattern.h"
#include "patterns/AudioTrianglesPattern.h"
#include "patterns/AuroraDropPattern.h"
#include "patterns/JuliaFractalPattern.h"
#include "patterns/RotatingSpectrumPattern.h"
#include "patterns/PhyllotaxisFractalPattern.h"
#include "patterns/DNAHelixPattern.h"
#include "patterns/SpectrumCirclePattern.h"
#include "patterns/Spectrum2Pattern.h"
#include "patterns/MorphingShapesPattern.h"

#include "playlists/MusicPlaylist.h"

static std::unique_ptr<MatrixPanel_I2S_DMA> dmaDisplay;
static Microphone mic;

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
        Serial.println("Failed to start MatrixDriver");
        ESP_INFINITE_LOOP();
    }
    Serial.println("Started MatrixDriver");

    mic.start();

    PatternRegistry::add<TestPattern>();
    PatternRegistry::add<TestPattern2>();
    PatternRegistry::add<MandalaPattern>();
    PatternRegistry::add<AudioSpectrumPattern>();
    PatternRegistry::add<AudioTrianglesPattern>();
    PatternRegistry::add<AuroraDropPattern>();
    PatternRegistry::add<JuliaFractalPattern>();
    PatternRegistry::add<RotatingSpectrumPattern>();
    PatternRegistry::add<PhyllotaxisFractalPattern>();
    PatternRegistry::add<DNAHelixPattern>();
    PatternRegistry::add<SpectrumCirclePattern>();
    PatternRegistry::add<Spectrum2Pattern>();
    PatternRegistry::add<MorphingShapesPattern>();

    PatternRegistry::add<MusicPlaylist>();
}

static uint32_t ms = 0;
static uint32_t fps = 0;
static bool didStartMusic = false;

static MatrixLeds leds;
static MatrixNoise noise;
static CRGBPalette16 currentPalette = PatternContext::randomPalette();
static CRGBPalette16 targetPalette = PatternContext::randomPalette();
static std::atomic<uint8_t> globalBrightness{255};

void loop()
{
    leds.clear();

    PatternContext ctx{
        leds,
        noise,
        mic.getContext(),
        currentPalette,
        targetPalette
    };

    if (!didStartMusic)
    {
        PatternRegistry::get(MusicPlaylist::ID)->start(ctx);
        didStartMusic = true;
    }

    nblendPaletteTowardPalette(currentPalette, targetPalette, 24);
    // PatternRegistry::get(MusicPlaylist::ID)->render(ctx);
    PatternRegistry::get(TestPattern2::ID)->render(ctx);

    dmaDisplay->setBrightness8(globalBrightness.load());

    for (size_t y = 0; y < dmaDisplay->height(); ++y)
    {
        for (size_t x = 0; x < dmaDisplay->width(); ++x)
        {
            const CRGB& led = leds(x, y);
            dmaDisplay->drawPixelRGB888(x, y, led.r, led.g, led.b);
        }
    }

    fps++;
    if (millis() - ms > 1000)
    {
        PatternRegistry::get(MandalaPattern::ID)->start(ctx);

        Serial.printf("FPS: %d\n", fps);
        ms = millis();
        fps = 0;
    }
}
