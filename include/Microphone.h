#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <complex>
#include <cstring>
#include <driver/i2s_std.h>
#include <driver/i2s_types.h>
#include <esp_err.h>
#include <ranges>

#include "BeatDetector.h"
#include "ThreadManager.h"

struct AudioContext
{
    std::array<uint8_t, BINS> heights8;
    std::array<uint8_t, BINS> peaks8;
    uint16_t bpm;
    bool isBeat;
    uint32_t totalBeats;
    float energy64f;
    float energy64fScaled;
    uint8_t energy8;
    uint8_t energy8Scaled;
    float energy64fPeaks;
    float energy64fPeaksScaled;
    uint8_t energy8Peaks;
    uint8_t energy8PeaksScaled;

    uint8_t avgHeights8Range(uint8_t low, uint8_t high) const
    {
        low = max<uint8_t>(low, 0);
        high = min<uint8_t>(high, BINS - 1);
        uint16_t sum = 0;
        for (uint8_t i = low; i < high; ++i)
            sum += heights8[i];
        return sum / (high - low);
    }

    uint8_t avgPeaks8Range(uint8_t low, uint8_t high) const
    {
        low = max<uint8_t>(low, 0);
        high = min<uint8_t>(high, BINS - 1);
        uint16_t sum = 0;
        for (uint8_t i = low; i < high; ++i)
            sum += peaks8[i];
        return sum / (high - low);
    }
};

class Microphone final
{
    static constexpr size_t SAMPLE_RATE = 22050;
    static constexpr size_t BUFFER_SIZE = 512;

    static constexpr float PEAK_HOLD_TIME = 3.0f;
    static constexpr float BAND_NORM_FACTOR = 0.995f;
    static constexpr float LOG_SCALE_BASE = 3.0f;
    static constexpr float ENERGY_ATTACK_FACTOR = 10.0f;
    static constexpr float ENERGY_ATTACK_MIN = 0.2f;
    static constexpr float ENERGY_ATTACK_MAX = 0.9f;
    static constexpr float ENERGY_DECAY_FACTOR = 0.15f;
    static constexpr float ENERGY_DECAY_MIN = 0.6f;
    static constexpr float ENERGY_DECAY_MAX = 0.95f;

    static constexpr gpio_num_t MIC_WS = GPIO_NUM_15;
    static constexpr gpio_num_t MIC_SCK = GPIO_NUM_14;
    static constexpr gpio_num_t MIC_SD = GPIO_NUM_32;

    std::mutex readMicMutex_;
    std::array<int32_t, BUFFER_SIZE> buffer_{};
    i2s_chan_handle_t rxChan_{};

    ThreadManager *processingThread_ = nullptr;
    std::atomic<bool> threadInitialized_{false};

    std::mutex spectrumMutex_;
    std::atomic<uint8_t> activeBuffer_{0};
    std::atomic<uint32_t> updateCount_{0};
    std::atomic<uint32_t> processingTimeUs_{0};
    std::atomic<uint32_t> micLastUpdateCount_{0};

    std::array<float, BINS> lastSpectrum_{};
    std::array<float, BINS> peakLevels_{};
    std::array<float, BINS> peakHoldCounters_{};
    std::array<float, BINS> bandMaxHistory_{};
    std::array<float, BINS> heightsBuffer_[2]{};
    std::array<float, BINS> peaksBuffer_[2]{};
    float energy_[2] = {0.0f, 0.0f};
    float energyPeaks_[2] = {0.0f, 0.0f};

    float dynAttack_ = 1.0f;
    float dynDecay_ = 1.0f;
    uint32_t totalBeats = 0;

    BeatDetector beatDetector_;

  public:
    void getContext(AudioContext &audio)
    {
        std::lock_guard lock(spectrumMutex_);

        if (updateCount_.load() == micLastUpdateCount_.load())
        {
            audio.isBeat = false;
            return;
        }

        micLastUpdateCount_.store(updateCount_.load());

        const uint8_t current_buffer = activeBuffer_.load();
        const auto &heights64f = heightsBuffer_[current_buffer];
        const auto &peaks64f = peaksBuffer_[current_buffer];

        std::array<uint8_t, BINS> heights8{};
        std::ranges::transform(
            heights64f, heights8.begin(), [](const float x) -> uint8_t { return 255.0f * x / 63.0f; });

        std::array<uint8_t, BINS> peaks8{};
        std::ranges::transform(peaks64f, peaks8.begin(), [](const float x) -> uint8_t { return 255.0f * x / 63.0f; });

        beatDetector_.update(heights64f);
        const bool isBeat = beatDetector_.isBeatDetected();
        if (isBeat)
            totalBeats++;

        audio.heights8 = heights8;
        audio.peaks8 = peaks8;
        audio.bpm = static_cast<uint16_t>(beatDetector_.getBPM());
        audio.isBeat = isBeat;
        audio.totalBeats = totalBeats;

        audio.energy64f = energy_[current_buffer];
        audio.energy64fScaled = min(63.0f, 1.5f * energy_[current_buffer]);
        audio.energy8 = 255.0f * energy_[current_buffer] / 63.0f;
        audio.energy8Scaled = min(255.0f, 1.5f * 255.0f * energy_[current_buffer] / 63.0f);

        audio.energy64fPeaks = energyPeaks_[current_buffer];
        audio.energy64fPeaksScaled = min(63.0f, 1.5f * energyPeaks_[current_buffer]);
        audio.energy8Peaks = 255.0f * energyPeaks_[current_buffer] / 63.0f;
        audio.energy8PeaksScaled = min(255.0f, 1.5f * 255.0f * energyPeaks_[current_buffer] / 63.0f);
    }

    void start()
    {
        Serial.println("Starting Microphone and FFT processing...");

        // Initialize spectrum buffers
        for (auto &buffer : heightsBuffer_)
        {
            buffer.fill(0.0f);
        }

        for (auto &buffer : peaksBuffer_)
        {
            buffer.fill(0.0f);
        }

        activeBuffer_.store(0);
        updateCount_.store(0);
        processingTimeUs_.store(0);
        threadInitialized_.store(false);

        constexpr i2s_chan_config_t chan_cfg = {
            .id = I2S_NUM_0,
            .role = I2S_ROLE_MASTER,
            .dma_desc_num = 8,
            .dma_frame_num = BUFFER_SIZE / 2,
            .auto_clear = true,
            .auto_clear_before_cb = true,
            .allow_pd = false,
            .intr_priority = 0,
        };

        esp_err_t err = i2s_new_channel(&chan_cfg, nullptr, &rxChan_);
        if (err != ESP_OK)
        {
            Serial.printf("Failed to create I2S RX channel: %s\n", esp_err_to_name(err));
            ESP_INFINITE_LOOP();
        }

        constexpr i2s_std_config_t std_cfg = {
            .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
            .gpio_cfg =
                {
                    .mclk = I2S_GPIO_UNUSED,
                    .bclk = MIC_SCK,
                    .ws = MIC_WS,
                    .dout = I2S_GPIO_UNUSED,
                    .din = MIC_SD,
                    .invert_flags =
                        {
                            .mclk_inv = false,
                            .bclk_inv = false,
                            .ws_inv = false,
                        },
                },
        };

        err = i2s_channel_init_std_mode(rxChan_, &std_cfg);
        if (err != ESP_OK)
        {
            Serial.printf("Failed to initialize I2S RX channel in STD mode: %s\n", esp_err_to_name(err));
            i2s_del_channel(rxChan_);
            ESP_INFINITE_LOOP();
        }

        err = i2s_channel_enable(rxChan_);
        if (err != ESP_OK)
        {
            Serial.printf("Failed to enable I2S RX channel: %s\n", esp_err_to_name(err));
            i2s_channel_disable(rxChan_);
            i2s_del_channel(rxChan_);
            ESP_INFINITE_LOOP();
        }

        static constexpr int FFT_THREAD_CORE = 0;
        static constexpr size_t FFT_THREAD_STACK_SIZE = 8192;
        static constexpr int FFT_THREAD_PRIORITY = 5; // Lower number = higher priority

        processingThread_ = new ThreadManager("MicFFT", FFT_THREAD_CORE, FFT_THREAD_STACK_SIZE, FFT_THREAD_PRIORITY);

        processingThread_->start([this](const std::atomic<bool> &running) { processingThreadFunc(running); });
        threadInitialized_.store(true);

        Serial.println("Microphone and FFT processing running");
    }

    void stop()
    {
        Serial.println("Destroying Microphone and FFT processing...");

        // Stop processing thread first
        if (threadInitialized_.load())
        {
            delete processingThread_;
            processingThread_ = nullptr;
            threadInitialized_.store(false);
        }

        // Then shutdown the I2S channel
        if (rxChan_)
        {
            i2s_channel_disable(rxChan_);
            i2s_del_channel(rxChan_);
        }

        Serial.println("Microphone and FFT processing destroyed");
    }

  private:
    static void fft(std::vector<std::complex<float>> &x)
    {
        const size_t N = x.size();
        if (N == 0)
            return;

        if ((N > 1) && (N & (N - 1)) != 0)
        {
            Serial.printf("FFT size %zu is not a power of 2. Aborting FFT.\n", N);
            return;
        }
        if (N <= 1)
            return;

        std::vector<std::complex<float>> scratch(N);
        fft_recursive_impl(x.data(), N, scratch.data());
    }

    static void fft_recursive_impl(std::complex<float> *x_data, const size_t N, std::complex<float> *scratch_data)
    {
        if (N <= 1)
            return;

        std::complex<float> *even_part_in_scratch = scratch_data;
        std::complex<float> *odd_part_in_scratch = scratch_data + N / 2;

        for (size_t i = 0; i < N / 2; i++)
        {
            even_part_in_scratch[i] = x_data[2 * i];
            odd_part_in_scratch[i] = x_data[2 * i + 1];
        }

        fft_recursive_impl(even_part_in_scratch, N / 2, x_data);
        fft_recursive_impl(odd_part_in_scratch, N / 2, x_data + N / 2);

        for (size_t k = 0; k < N / 2; k++)
        {
            const float angle = -2.0f * M_PI * static_cast<float>(k) / static_cast<float>(N);
            const std::complex<float> t = std::polar(1.0f, angle) * odd_part_in_scratch[k];

            x_data[k] = even_part_in_scratch[k] + t;
            x_data[k + N / 2] = even_part_in_scratch[k] - t;
        }
    }

    void processingThreadFunc(const std::atomic<bool> &running)
    {
        std::vector<std::complex<float>> fft_input(BUFFER_SIZE);
        std::array<int32_t, BUFFER_SIZE> local_buffer{};
        std::array<float, BINS> local_spectrum{};
        std::array<float, BINS> local_heights{};
        std::array<float, BINS> local_peaks{};

        Serial.printf("FFT processing thread started on core %d\n", xPortGetCoreID());

        uint32_t last_process_time = 0;

        while (running)
        {
            // Don't process too frequently to avoid CPU overload
            if (constexpr uint32_t min_interval_ms = 30;
                esp_timer_get_time() / 1000 - last_process_time < min_interval_ms)
            {
                vTaskDelay(pdMS_TO_TICKS(5));
                continue;
            }

            // Read microphone data with timeout
            size_t bytes_read = 0;
            {
                std::lock_guard lock(readMicMutex_);
                const esp_err_t err = i2s_channel_read(
                    rxChan_,
                    buffer_.data(),
                    BUFFER_SIZE * sizeof(int32_t),
                    &bytes_read,
                    100 / portTICK_PERIOD_MS); // 100ms timeout

                if (err != ESP_OK)
                {
                    // Just skip this cycle if there's an error
                    vTaskDelay(pdMS_TO_TICKS(10));
                    continue;
                }

                // Copy to local buffer for processing
                std::memcpy(local_buffer.data(), buffer_.data(), sizeof(local_buffer));
            }

            if (bytes_read < BUFFER_SIZE * sizeof(int32_t))
            {
                // Pad with zeros if we didn't get a full buffer
                for (size_t i = bytes_read / sizeof(int32_t); i < BUFFER_SIZE; ++i)
                {
                    local_buffer[i] = 0;
                }
            }

            const auto start_time = esp_timer_get_time();

            // Prepare FFT input with window function
            for (size_t i = 0; i < BUFFER_SIZE; ++i)
            {
                const float cos_res = cosf(2.0f * M_PI * static_cast<float>(i) / static_cast<float>(BUFFER_SIZE - 1));
                const float window_val = 0.5f * (1.0f - cos_res);
                const float sample_float = static_cast<float>(local_buffer[i] >> 16) / 32768.0f;
                fft_input[i] = std::complex(sample_float * window_val, 0.0f);
            }

            // Perform FFT
            fft(fft_input);

            float energy = 0;

            // Calculate magnitude spectrum
            for (size_t i = 0; i < BINS; ++i)
            {
                const float real_part = fft_input[i].real();
                const float imag_part = fft_input[i].imag();
                local_spectrum[i] = std::sqrtf(real_part * real_part + imag_part * imag_part);

                // Apply logarithmic scaling and per-band normalization
                const float scaledValue = 1.0f + local_spectrum[i] * LOG_SCALE_BASE;
                local_spectrum[i] = logf(scaledValue) / logf(1.0f + LOG_SCALE_BASE);

                // Update band max history and normalize
                bandMaxHistory_[i] = std::max(bandMaxHistory_[i] * BAND_NORM_FACTOR, local_spectrum[i]);
                const float normFactor = std::max(0.01f, bandMaxHistory_[i]);
                local_spectrum[i] = local_spectrum[i] / normFactor;

                // Scale to matrix height
                local_spectrum[i] *= BINS - 1;
                energy += local_spectrum[i];
            }

            energy /= BINS;
            dynAttack_ = 1.0f + energy * ENERGY_ATTACK_FACTOR;
            dynDecay_ = 1.0f - energy * ENERGY_DECAY_FACTOR;
            dynAttack_ = std::min(std::max(dynAttack_, ENERGY_ATTACK_MIN), ENERGY_ATTACK_MAX);
            dynDecay_ = std::min(std::max(dynDecay_, ENERGY_DECAY_MIN), ENERGY_DECAY_MAX);

            energy = 0;
            float energyPeaks = 0;

            for (uint8_t x = 0; x < BINS; ++x)
            {
                if (const float currentValue = local_spectrum[x]; currentValue > lastSpectrum_[x])
                {
                    lastSpectrum_[x] = lastSpectrum_[x] * (1.0f - dynAttack_) + currentValue * dynAttack_;
                }
                else
                {
                    lastSpectrum_[x] = lastSpectrum_[x] * dynDecay_ + currentValue * (1.0f - dynDecay_);
                }

                if (lastSpectrum_[x] > peakLevels_[x])
                {
                    peakLevels_[x] = lastSpectrum_[x];
                    peakHoldCounters_[x] = PEAK_HOLD_TIME;
                }
                else
                {
                    if (peakHoldCounters_[x] > 0)
                    {
                        peakHoldCounters_[x] -= 1.0f;
                    }
                    else
                    {
                        peakLevels_[x] *= 0.9f;
                    }
                }

                local_heights[x] = std::min(lastSpectrum_[x], static_cast<float>(BINS));
                energy += local_heights[x];
                local_peaks[x] = std::min(peakLevels_[x], static_cast<float>(BINS));
                energyPeaks += local_peaks[x];
            }

            energy /= BINS;
            energyPeaks /= BINS;

            // Track processing time
            processingTimeUs_.store(esp_timer_get_time() - start_time);
            last_process_time = esp_timer_get_time() / 1000;

            // Store results in inactive buffer
            const uint8_t write_buffer = 1 - activeBuffer_.load();
            {
                std::lock_guard lock(spectrumMutex_);
                std::memcpy(heightsBuffer_[write_buffer].data(), local_heights.data(), sizeof(float) * BINS);
                std::memcpy(peaksBuffer_[write_buffer].data(), local_peaks.data(), sizeof(float) * BINS);
                energy_[write_buffer] = energy;
                energyPeaks_[write_buffer] = energyPeaks;
            }

            // Switch active buffer
            activeBuffer_.store(write_buffer);
            updateCount_.fetch_add(1);

            // Yield to other tasks
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        Serial.println("FFT processing thread ended");
    }
};
