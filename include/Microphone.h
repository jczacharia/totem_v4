#pragma once

#include <complex>
#include <driver/i2s_types.h>
#include <driver/i2s_std.h>
#include <array>
#include <atomic>
#include <cstring>
#include <esp_err.h>
#include <ranges>
#include <algorithm>

#include "ThreadManager.h"
#include "BeatDetector.h"

struct AudioContext
{
    std::array<uint8_t, MATRIX_WIDTH> heights8;
    std::array<uint8_t, MATRIX_WIDTH> peaks8;
    uint16_t bpm;
    bool isBeat;
    uint32_t totalBeats;
    float energy64f;
    float energy64fScaled;
    uint8_t energy8;
    uint8_t energy8Scaled;
};

class Microphone final
{
    static constexpr size_t SAMPLE_RATE = 22050;
    static constexpr size_t BUFFER_SIZE = 512;

    static constexpr float PEAK_HOLD_TIME = 3.0f;
    static constexpr float BAND_NORM_FACTOR = 0.995f;
    static constexpr float LOG_SCALE_BASE = 8.0f;
    static constexpr float ENERGY_ATTACK_FACTOR = 10.0f;
    static constexpr float ENERGY_ATTACK_MIN = 0.2f;
    static constexpr float ENERGY_ATTACK_MAX = 0.9f;
    static constexpr float ENERGY_DECAY_FACTOR = 0.15f;
    static constexpr float ENERGY_DECAY_MIN = 0.6f;
    static constexpr float ENERGY_DECAY_MAX = 0.95f;

    static constexpr gpio_num_t MIC_WS = GPIO_NUM_15;
    static constexpr gpio_num_t MIC_SCK = GPIO_NUM_14;
    static constexpr gpio_num_t MIC_SD = GPIO_NUM_32;

    std::mutex read_mic_mutex_;
    std::array<int32_t, BUFFER_SIZE> buffer_{};
    i2s_chan_handle_t rx_chan_{};

    ThreadManager* processing_thread_ = nullptr;
    std::atomic<bool> thread_initialized_{false};

    std::mutex spectrum_mutex_;
    std::atomic<uint8_t> active_buffer_{0};
    std::atomic<uint32_t> update_count_{0};
    std::atomic<uint32_t> processing_time_us_{0};
    std::atomic<uint32_t> mic_last_update_count_{0};

    std::array<float, MATRIX_WIDTH> lastSpectrum_{};
    std::array<float, MATRIX_WIDTH> peakLevels_{};
    std::array<float, MATRIX_WIDTH> peakHoldCounters_{};
    std::array<float, MATRIX_WIDTH> bandMaxHistory_{};
    std::array<float, MATRIX_WIDTH> heights_buffer_[2]{};
    std::array<float, MATRIX_WIDTH> peaks_buffer_[2]{};
    float energy_[2] = {0.0f, 0.0f};

    float dyn_attack_ = 1.0f;
    float dyn_decay_ = 1.0f;
    uint32_t totalBeats = 0;

    BeatDetector beat_detector_;
    AudioContext audio_context_{};

public:
    const AudioContext& getContext()
    {
        std::lock_guard lock(spectrum_mutex_);

        if (update_count_.load() == mic_last_update_count_.load())
        {
            audio_context_.isBeat = false;
            return audio_context_;
        }

        mic_last_update_count_.store(update_count_.load());

        const uint8_t current_buffer = active_buffer_.load();
        const auto& heights64f = heights_buffer_[current_buffer];
        const auto& peaks64f = peaks_buffer_[current_buffer];

        std::array<uint8_t, MATRIX_WIDTH> heights8{};
        std::ranges::transform(heights64f, heights8.begin(),
                               [](const float x) -> uint8_t { return 255.0f * x / 63.0f; });

        std::array<uint8_t, MATRIX_WIDTH> peaks8{};
        std::ranges::transform(peaks64f, peaks8.begin(),
                               [](const float x) -> uint8_t { return 255.0f * x / 63.0f; });

        beat_detector_.update(heights64f);
        const bool isBeat = beat_detector_.isBeatDetected();
        if (isBeat) totalBeats++;

        audio_context_.heights8 = heights8;
        audio_context_.peaks8 = peaks8;
        audio_context_.bpm = static_cast<uint16_t>(beat_detector_.getBPM());
        audio_context_.isBeat = isBeat;
        audio_context_.totalBeats = totalBeats;
        audio_context_.energy64f = energy_[current_buffer];
        audio_context_.energy64fScaled = min(63.0f, 1.5f * energy_[current_buffer]);
        audio_context_.energy8 = 255.0f * energy_[current_buffer] / 63.0f;
        audio_context_.energy8Scaled = min(255.0f, 1.5f * 255.0f * energy_[current_buffer] / 63.0f);

        return audio_context_;
    }

    void start()
    {
        Serial.println("Starting Microphone and FFT processing...");

        // Initialize spectrum buffers
        for (auto& buffer : heights_buffer_)
        {
            buffer.fill(0.0f);
        }

        for (auto& buffer : peaks_buffer_)
        {
            buffer.fill(0.0f);
        }

        active_buffer_.store(0);
        update_count_.store(0);
        processing_time_us_.store(0);
        thread_initialized_.store(false);

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

        esp_err_t err = i2s_new_channel(&chan_cfg, nullptr, &rx_chan_);
        if (err != ESP_OK)
        {
            Serial.printf("Failed to create I2S RX channel: %s\n", esp_err_to_name(err));
            ESP_INFINITE_LOOP();
        }

        constexpr i2s_std_config_t std_cfg = {
            .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
            .gpio_cfg = {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = MIC_SCK,
                .ws = MIC_WS,
                .dout = I2S_GPIO_UNUSED,
                .din = MIC_SD,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv = false,
                },
            },
        };

        err = i2s_channel_init_std_mode(rx_chan_, &std_cfg);
        if (err != ESP_OK)
        {
            Serial.printf("Failed to initialize I2S RX channel in STD mode: %s\n", esp_err_to_name(err));
            i2s_del_channel(rx_chan_);
            ESP_INFINITE_LOOP();
        }

        err = i2s_channel_enable(rx_chan_);
        if (err != ESP_OK)
        {
            Serial.printf("Failed to enable I2S RX channel: %s\n", esp_err_to_name(err));
            i2s_channel_disable(rx_chan_);
            i2s_del_channel(rx_chan_);
            ESP_INFINITE_LOOP();
        }

        static constexpr int FFT_THREAD_CORE = 0;
        static constexpr size_t FFT_THREAD_STACK_SIZE = 8192;
        static constexpr int FFT_THREAD_PRIORITY = 5; // Lower number = higher priority

        processing_thread_ = new ThreadManager(
            "MicFFT", FFT_THREAD_CORE,
            FFT_THREAD_STACK_SIZE,
            FFT_THREAD_PRIORITY);

        processing_thread_->start([this](const std::atomic<bool>& running) { processingThreadFunc(running); });
        thread_initialized_.store(true);

        Serial.println("Microphone and FFT processing running");
    }

    void stop()
    {
        Serial.println("Destroying Microphone and FFT processing...");

        // Stop processing thread first
        if (thread_initialized_.load())
        {
            delete processing_thread_;
            processing_thread_ = nullptr;
            thread_initialized_.store(false);
        }

        // Then shutdown the I2S channel
        if (rx_chan_)
        {
            i2s_channel_disable(rx_chan_);
            i2s_del_channel(rx_chan_);
        }

        Serial.println("Microphone and FFT processing destroyed");
    }

private:
    static void fft(std::vector<std::complex<float>>& x)
    {
        const size_t N = x.size();
        if (N == 0) return;

        if ((N > 1) && (N & (N - 1)) != 0)
        {
            Serial.printf("FFT size %zu is not a power of 2. Aborting FFT.\n", N);
            return;
        }
        if (N <= 1) return;

        std::vector<std::complex<float>> scratch(N);
        fft_recursive_impl(x.data(), N, scratch.data());
    }

    static void fft_recursive_impl(std::complex<float>* x_data, const size_t N, std::complex<float>* scratch_data)
    {
        if (N <= 1) return;

        std::complex<float>* even_part_in_scratch = scratch_data;
        std::complex<float>* odd_part_in_scratch = scratch_data + N / 2;

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

    void processingThreadFunc(const std::atomic<bool>& running)
    {
        std::vector<std::complex<float>> fft_input(BUFFER_SIZE);
        std::array<int32_t, BUFFER_SIZE> local_buffer{};
        std::array<float, MATRIX_WIDTH> local_spectrum{};
        std::array<float, MATRIX_WIDTH> local_heights{};
        std::array<float, MATRIX_WIDTH> local_peaks{};

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
                std::lock_guard lock(read_mic_mutex_);
                const esp_err_t err = i2s_channel_read(rx_chan_, buffer_.data(),
                                                       BUFFER_SIZE * sizeof(int32_t),
                                                       &bytes_read, 100 / portTICK_PERIOD_MS); // 100ms timeout

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
            for (size_t i = 0; i < MATRIX_WIDTH; ++i)
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
                local_spectrum[i] *= (MATRIX_WIDTH - 1);
                energy += local_spectrum[i];
            }

            energy /= (MATRIX_WIDTH - 1);
            dyn_attack_ = 1.0f + energy * ENERGY_ATTACK_FACTOR;
            dyn_decay_ = 1.0f - energy * ENERGY_DECAY_FACTOR;
            dyn_attack_ = std::min(std::max(dyn_attack_, ENERGY_ATTACK_MIN), ENERGY_ATTACK_MAX);
            dyn_decay_ = std::min(std::max(dyn_decay_, ENERGY_DECAY_MIN), ENERGY_DECAY_MAX);

            energy = 0;

            for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
            {
                if (const float currentValue = local_spectrum[x]; currentValue > lastSpectrum_[x])
                {
                    lastSpectrum_[x] = lastSpectrum_[x] * (1.0f - dyn_attack_) + currentValue * dyn_attack_;
                }
                else
                {
                    lastSpectrum_[x] = lastSpectrum_[x] * dyn_decay_ + currentValue * (1.0f - dyn_decay_);
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

                local_heights[x] = std::min(lastSpectrum_[x], static_cast<float>(MATRIX_WIDTH));
                local_peaks[x] = std::min(peakLevels_[x], static_cast<float>(MATRIX_WIDTH - 1));
                energy += local_heights[x];
            }

            energy /= (MATRIX_WIDTH - 1);

            // Track processing time
            processing_time_us_.store(esp_timer_get_time() - start_time);
            last_process_time = esp_timer_get_time() / 1000;

            // Store results in inactive buffer
            const uint8_t write_buffer = 1 - active_buffer_.load();
            {
                std::lock_guard lock(spectrum_mutex_);
                std::memcpy(heights_buffer_[write_buffer].data(), local_heights.data(), sizeof(float) * MATRIX_WIDTH);
                std::memcpy(peaks_buffer_[write_buffer].data(), local_peaks.data(), sizeof(float) * MATRIX_WIDTH);
                energy_[write_buffer] = energy;
            }

            // Switch active buffer
            active_buffer_.store(write_buffer);
            update_count_.fetch_add(1);

            // Yield to other tasks
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        Serial.println("FFT processing thread ended");
    }
};
