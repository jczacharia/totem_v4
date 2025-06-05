#pragma once

#include <array>
#include <esp_timer.h>

class BeatDetector
{
    // Constants for beat detection
    static constexpr size_t HISTORY_SIZE = 64;
    static constexpr float SENSITIVITY = 1.2f;        // Lowered from 1.5 to be more sensitive
    static constexpr uint32_t MIN_INTERVAL_MS = 300;  // Increased minimum time between beats
    static constexpr uint32_t MAX_INTERVAL_MS = 2000; // Maximum time between beats (30 BPM)

    // Energy and beat tracking
    std::array<float, HISTORY_SIZE> energy_history_{};
    size_t history_index_ = 0;

    // Time tracking for beats
    uint32_t last_beat_time_ = 0;
    std::array<uint32_t, 12> beat_intervals_{}; // Store more intervals for better averaging
    size_t beat_interval_index_ = 0;

    // Current BPM value
    float current_bpm_ = 0.0f;

    // Beat detection flag
    bool beat_detected_ = false;

    // Beat energy for visualization
    float current_beat_energy_ = 0.0f;

    // Debug counters
    uint32_t total_beats_ = 0;
    uint32_t start_time_ = 0;

  public:
    BeatDetector()
    {
        energy_history_.fill(0);
        beat_intervals_.fill(0);
        start_time_ = esp_timer_get_time() / 1000;
    }

    // Called regularly with new spectrum data
    void update(const std::array<float, MATRIX_WIDTH> &spectrum)
    {
        // Reset beat detection flag at the start of each update
        beat_detected_ = false;

        // For 64 bins and 22050Hz sample rate and selecting bins 10-50
        // 10     - 50
        // 3445Hz - 17226Hz
        float current_energy = 0;

        for (size_t i = 10; i < 50; i++)
        {
            current_energy += spectrum[i];
        }

        // Store energy in history
        energy_history_[history_index_] = current_energy;

        // Calculate local average and standard deviation
        float avg_energy = 0;
        for (const auto &e : energy_history_)
        {
            avg_energy += e;
        }
        avg_energy /= HISTORY_SIZE;

        // Calculate standard deviation
        float variance = 0;
        for (const auto &e : energy_history_)
        {
            const float diff = e - avg_energy;
            variance += diff * diff;
        }
        variance /= HISTORY_SIZE;
        const float std_dev = std::sqrt(variance);

        // Detect beat - energy spike relative to recent history
        const uint32_t current_time = esp_timer_get_time() / 1000;

        // Using standard deviation-based threshold for dynamic sensitivity
        const float threshold = avg_energy + (std_dev * SENSITIVITY);

        // Store current beat energy for visualization
        current_beat_energy_ = current_energy / (threshold + 0.001f); // Normalized energy level

        // Check if current energy exceeds threshold and enough time has passed since last beat
        if (current_energy > threshold && (current_time - last_beat_time_ > MIN_INTERVAL_MS) &&
            current_energy > 0.1)
        {
            // Small minimum threshold to avoid noise
            beat_detected_ = true;
            total_beats_++;

            // Calculate time since last beat
            if (last_beat_time_ > 0)
            {
                // Only use reasonable intervals
                if (const uint32_t interval = current_time - last_beat_time_; interval < MAX_INTERVAL_MS)
                {
                    // Store interval
                    beat_intervals_[beat_interval_index_] = interval;
                    beat_interval_index_ = (beat_interval_index_ + 1) % beat_intervals_.size();

                    // Calculate BPM using median filtering
                    calculateBPM();
                }
            }

            last_beat_time_ = current_time;
        }

        // Update history index
        history_index_ = (history_index_ + 1) % HISTORY_SIZE;
    }

    // Check if a beat was detected in the current update cycle
    [[nodiscard]] bool isBeatDetected() const
    {
        return beat_detected_;
    }

    // Get the normalized beat energy (useful for visualizations)
    [[nodiscard]] float getBeatEnergy() const
    {
        return current_beat_energy_;
    }

    // Get the time since the last beat in milliseconds
    [[nodiscard]] uint32_t getTimeSinceLastBeat() const
    {
        if (last_beat_time_ == 0)
        {
            return 0;
        }

        return (esp_timer_get_time() / 1000) - last_beat_time_;
    }

    // Get the current BPM estimate
    [[nodiscard]] float getBPM() const
    {
        return current_bpm_;
    }

    // Get percentage confidence in the current BPM
    [[nodiscard]] float getConfidence() const
    {
        // Simple confidence measure based on number of intervals collected
        float confidence = 0;
        for (const auto &interval : beat_intervals_)
        {
            if (interval > 0)
            {
                confidence += 1.0f;
            }
        }

        return (confidence / beat_intervals_.size()) * 100.0f;
    }

  private:
    void calculateBPM()
    {
        // Copy valid intervals to a temporary array
        std::array<uint32_t, 12> valid_intervals{};
        size_t valid_count = 0;

        for (const auto &interval : beat_intervals_)
        {
            if (interval > 0 && interval < MAX_INTERVAL_MS)
            {
                valid_intervals[valid_count++] = interval;
            }
        }

        if (valid_count < 3)
            return; // Need at least 3 intervals for reliable BPM

        // Sort intervals for median calculation
        for (size_t i = 0; i < valid_count - 1; i++)
        {
            for (size_t j = 0; j < valid_count - i - 1; j++)
            {
                if (valid_intervals[j] > valid_intervals[j + 1])
                {
                    std::swap(valid_intervals[j], valid_intervals[j + 1]);
                }
            }
        }

        // Use median interval for BPM calculation to reduce outlier impact
        uint32_t median_interval;
        if (valid_count % 2 == 0)
        {
            median_interval = (valid_intervals[valid_count / 2 - 1] + valid_intervals[valid_count / 2]) / 2;
        }
        else
        {
            median_interval = valid_intervals[valid_count / 2];
        }

        // Convert to BPM
        const float new_bpm = 60000.0f / median_interval;

        // Apply smoothing - gradually adjust current BPM towards new value
        if (current_bpm_ > 0)
        {
            current_bpm_ = current_bpm_ * 0.7f + new_bpm * 0.3f;
        }
        else
        {
            current_bpm_ = new_bpm;
        }

        // Constrain to reasonable BPM range for music
        if (current_bpm_ > 0)
        {
            // Adjust for tempo octave errors
            while (current_bpm_ < 60)
            {
                current_bpm_ *= 2;
            }
            while (current_bpm_ > 180)
            {
                current_bpm_ /= 2;
            }
        }
    }
};
