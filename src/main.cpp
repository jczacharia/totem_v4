#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <GFX_Layer.hpp>
#include <GFX_Lite.h>
#include <WiFi.h>
#include <atomic>
#include <cstring>

#include "MatrixGfx.h"
#include "MatrixNoise.h"
#include "Microphone.h"
#include "Registry.h"
#include "Util.h"
#include "index.h"

#include "patterns/Audio2dGridPattern.h"
#include "patterns/Audio2dWavesPattern.h"
#include "patterns/Audio3dGridPattern.h"
#include "patterns/Audio8x8SquaresPattern.h"
#include "patterns/AudioClassicSpectrum128Pattern.h"
#include "patterns/AudioCubesPattern.h"
#include "patterns/AudioDiagonalSpectrumPattern.h"
#include "patterns/AudioDotsSinglePattern.h"
#include "patterns/AudioSpectrumPattern.h"
#include "patterns/AudioTrianglesPattern.h"
#include "patterns/AuroraDropPattern.h"
#include "patterns/BigSparkPattern.h"
#include "patterns/CirclesPattern.h"
#include "patterns/DNAHelixPattern.h"
#include "patterns/FlowerOfLifePattern.h"
#include "patterns/JuliaFractalPattern.h"
#include "patterns/LifePattern.h"
#include "patterns/Mandala2Pattern.h"
#include "patterns/MandalaPattern.h"
#include "patterns/MetatronsCubePattern.h"
#include "patterns/MorphingShapesPattern.h"
#include "patterns/MunchPattern.h"
#include "patterns/NoopPattern.h"
#include "patterns/PhyllotaxisFractalPattern.h"
#include "patterns/PlasmaPattern.h"
#include "patterns/RorschachPattern.h"
#include "patterns/RotatingSpectrumPattern.h"
#include "patterns/SimpleNoisePattern.h"
#include "patterns/Spectrum2Pattern.h"
#include "patterns/SpectrumCirclePattern.h"
#include "patterns/SpectrumPeakBarsPattern.h"
#include "patterns/TorusPattern.h"

#include "playlists/MusicPlaylist.h"

static Microphone mic;
static std::unique_ptr<MatrixPanel_I2S_DMA> dmaDisplay;

static AsyncWebServer server(80);
static DNSServer dnsServer;
static constexpr byte DNS_PORT = 53;

enum class TotemState
{
    MUSIC,
    GIF,
    PATTERN,
};

static std::atomic currentTotemState{TotemState::MUSIC};
static std::mutex stateMutex;
static std::atomic<uint8_t> globalBrightness{255};

static std::shared_ptr<Pattern> patternState;

static auto commandEndpoint = new AsyncCallbackJsonWebHandler("/command");
static auto brightnessEndpoint = new AsyncCallbackJsonWebHandler("/brightness");
static auto getPatternIdsEndpoint = new AsyncCallbackJsonWebHandler("/patterns");

static constexpr auto MATRIX_BUFFER_SIZE = MATRIX_WIDTH * MATRIX_HEIGHT * sizeof(uint32_t);
static std::atomic<uint16_t> currentGifFrameIdx = 0;
static std::vector<uint8_t> gif;
static std::vector<uint8_t> gifBuf;
static constexpr std::array<size_t, 256> DRAM_ATTR GIF_GAMMA = GenGammaTable<1.8f, 256, 255>();

static void setupEndpoints()
{
    // Command endpoint
    commandEndpoint->setMethod(HTTP_POST);
    commandEndpoint->onRequest(
        [](AsyncWebServerRequest *request, const JsonVariant &json)
        {
            if (const String cmd = json["cmd"]; cmd == "ping")
            {
                request->send(200, "text/plain", "pong");
            }
            else if (cmd == "music")
            {
                std::lock_guard lock(stateMutex);
                currentTotemState.store(TotemState::MUSIC);
                globalBrightness.store(200);
                request->send(200);
            }
            else if (cmd == "pattern")
            {
                std::lock_guard lock(stateMutex);
                currentTotemState.store(TotemState::PATTERN);
                patternState = Registry::get(json["id"]);
                if (patternState)
                {
                    patternState->start();
                    request->send(200);
                }
                else
                {
                    request->send(400, "text/plain", "Unknown pattern");
                }
            }
            else
            {
                request->send(400, "text/plain", "Unknown command");
            }
        });
    server.addHandler(commandEndpoint);

    // Brightness endpoint
    brightnessEndpoint->setMethod(HTTP_POST);
    brightnessEndpoint->onRequest(
        [](AsyncWebServerRequest *request, const JsonVariant &json)
        {
            std::lock_guard lock(stateMutex);
            globalBrightness.store(json["value"]);
            request->send(200);
        });
    server.addHandler(brightnessEndpoint);

    // Get Pattern Ids endpoint
    getPatternIdsEndpoint->setMethod(HTTP_GET);
    getPatternIdsEndpoint->onRequest(
        [](AsyncWebServerRequest *request, const JsonVariant &json)
        {
            JsonDocument doc;
            JsonArray jsonArray = doc.to<JsonArray>(); // Better syntax
            for (auto id : Registry::getAllIds())
            {
                jsonArray.add(id);
            }
            String jsonString;
            serializeJson(doc, jsonString); // This is correct - serialize the document
            request->send(200, "application/json", jsonString);
        });
    server.addHandler(getPatternIdsEndpoint);

    // GIF upload endpoint
    server.on(
        "/gif",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            std::lock_guard lock(stateMutex);
            request->send(200, "text/plain", "GIF uploaded successfully");
            Serial.printf(
                "GIF mode set with total len of %d with frames %d\n",
                gifBuf.size(),
                gifBuf.size() / MATRIX_BUFFER_SIZE);
            currentGifFrameIdx.store(0);
            gif = std::move(gifBuf);
            globalBrightness.store(80);
            currentTotemState.store(TotemState::GIF);
        },
        nullptr,
        [](AsyncWebServerRequest *request,
           const uint8_t *data,
           const size_t len,
           const size_t index,
           const size_t total)
        {
            if (index == 0)
            {
                const auto free_mem = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
                Serial.printf("Receiving GIF: %d bytes, heap available: %d\n", total, free_mem);

                if (total == 0 || total > free_mem || total < MATRIX_BUFFER_SIZE || total % MATRIX_BUFFER_SIZE != 0)
                {
                    request->send(400, "text/plain", "Invalid GIF size");
                    request->abort();
                    return;
                }

                gifBuf.resize(total);
            }

            std::memcpy(gifBuf.data() + index, data, len);
        });
}

class CaptiveRequestHandler final : public AsyncWebHandler
{
  public:
    bool canHandle(AsyncWebServerRequest *request) const override
    {
        // Handle all requests
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request) override
    {
        // Redirect to our main page if accessing a different URL
        if (request->url() != "/" && request->url() != "/index.html")
        {
            request->redirect("/");
        }
        else
        {
            request->send(200, "text/html", index_html);
        }
    }
};

static void startServer()
{
    // Set up WiFi Access Point
    WiFiClass::mode(WIFI_AP);
    WiFi.softAP("Square Rats Totem"); // No password for easier access

    const IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Set up DNS server to redirect all requests to us
    dnsServer.start(DNS_PORT, "*", IP);

    // Set up web server endpoints
    setupEndpoints();

    // Add captive portal handler last (catches all other requests)
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

    // Start server
    server.begin();
}

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
    mic.getContext(Pattern::Audio);
    random16_set_seed(UINT16_MAX * Pattern::Audio.energy8 / 255);

    Registry::add<Audio2dGridPattern>();
    Registry::add<Audio2dWavesPattern>();
    Registry::add<Audio3dGridPattern>();
    Registry::add<Audio8x8SquaresPattern>();
    Registry::add<AudioClassicSpectrum128Pattern>();
    Registry::add<AudioCubesPattern>();
    Registry::add<AudioDiagonalSpectrumPattern>();
    Registry::add<AudioDotsSinglePattern>();
    Registry::add<AudioSpectrumPattern>();
    Registry::add<AudioTrianglesPattern>();
    Registry::add<AuroraDropPattern>();
    Registry::add<BigSparkPattern>();
    Registry::add<CirclesPattern>();
    Registry::add<DNAHelixPattern>();
    Registry::add<FlowerOfLifePattern>();
    Registry::add<JuliaFractalPattern>();
    Registry::add<LifePattern>();
    Registry::add<Mandala2Pattern>();
    Registry::add<MandalaPattern>();
    Registry::add<MetatronsCubePattern>();
    Registry::add<MorphingShapesPattern>();
    Registry::add<MunchPattern>();
    Registry::add<NoopPattern>();
    Registry::add<PhyllotaxisFractalPattern>();
    Registry::add<PlasmaPattern>();
    Registry::add<RorschachPattern>();
    Registry::add<RotatingSpectrumPattern>();
    Registry::add<SimpleNoisePattern>();
    Registry::add<Spectrum2Pattern>();
    Registry::add<SpectrumCirclePattern>();
    Registry::add<SpectrumPeakBarsPattern>();
    Registry::add<TorusPattern>();

    Registry::add<MusicPlaylist>();
    Registry::get(MusicPlaylist::ID)->start();

    startServer();
}

static uint32_t ms = 0;
static uint32_t fps = 0;

void loop()
{
    dnsServer.processNextRequest();
    std::lock_guard lock(stateMutex);

    Pattern::clearAllGfx();
    mic.getContext(Pattern::Audio);
    Pattern::updateBpmOscillators(Pattern::Audio.bpm);
    random16_set_seed(UINT16_MAX * Pattern::Audio.energy64f / 63.0f);

    switch (currentTotemState.load())
    {
        case TotemState::GIF:
        {
            if (gif.empty())
            {
                Serial.println("Gif buf is empty! Switching to music mode.");
                currentTotemState.store(TotemState::MUSIC);
                return;
            }

            const size_t frameOffset = currentGifFrameIdx * (MATRIX_WIDTH * MATRIX_HEIGHT);
            const auto *gBuf = reinterpret_cast<uint32_t *>(gif.data());

            for (int16_t y = 0; y < MATRIX_HEIGHT; ++y)
            {
                for (int16_t x = 0; x < MATRIX_WIDTH; ++x)
                {
                    const CRGB &led = gBuf[frameOffset + y * MATRIX_WIDTH + x];
                    Pattern::Gfx(x, y) = CRGB(GIF_GAMMA[led.r], GIF_GAMMA[led.g], GIF_GAMMA[led.b]);
                }
            }

            currentGifFrameIdx = (currentGifFrameIdx + 1) % (gif.size() / MATRIX_BUFFER_SIZE);
            delay(60);
        }
        break;
        case TotemState::MUSIC:
        {
            Registry::get(MusicPlaylist::ID)->render();
        }
        break;
        case TotemState::PATTERN:
        {
            if (patternState)
                patternState->render();
        }
        break;
    }

    dmaDisplay->setBrightness8(globalBrightness.load());
    for (int16_t y = 0; y < dmaDisplay->height(); ++y)
    {
        for (int16_t x = 0; x < dmaDisplay->width(); ++x)
        {
            const CRGB &led = Pattern::Gfx(x, y) + Pattern::GfxBkg(x, y);
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
