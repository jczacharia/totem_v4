#pragma once

template <float TGamma, size_t TSteps, size_t TMaxVal>
constexpr std::array<size_t, TSteps> GenGammaTable()
{
    const float t_max = static_cast<float>(TMaxVal);
    std::array<size_t, TSteps> table{};
    for (size_t i = 0; i < TSteps; ++i)
    {
        const float normalized = static_cast<float>(i) / (TSteps - 1);
        const float corrected = std::pow(normalized, TGamma);
        table[i] = static_cast<size_t>(std::lround(corrected * t_max));
    }
    return table;
}

FORCE_INLINE_ATTR uint8_t atan2_8(int16_t dy, int16_t dx)
{
    if (dx == 0 && dy == 0)
        return 0;

    // Use a simple approach based on dx and dy signs and magnitudes
    // This avoids sharp transitions at quadrant boundaries

    // Normalize to prevent overflow
    while (abs(dx) > 127 || abs(dy) > 127)
    {
        dx >>= 1;
        dy >>= 1;
    }

    // Convert to 0-255 range using a continuous approximation
    // This method reduces discontinuities significantly
    uint16_t angle = 0;

    if (dx >= 0)
    {
        if (dy >= 0)
        {
            // Q1: 0-90 degrees (0-64 in our scale)
            if (dx >= dy)
                angle = (dy * 32) / (dx + 1);
            else
                angle = 64 - (dx * 32) / (dy + 1);
        }
        else
        {
            // Q4: 270-360 degrees (192-256 in our scale)
            dy = -dy;
            if (dx >= dy)
                angle = 256 - (dy * 32) / (dx + 1);
            else
                angle = 192 + (dx * 32) / (dy + 1);
        }
    }
    else
    {
        dx = -dx;
        if (dy >= 0)
        {
            // Q2: 90-180 degrees (64-128 in our scale)
            if (dx >= dy)
                angle = 128 - (dy * 32) / (dx + 1);
            else
                angle = 64 + (dx * 32) / (dy + 1);
        }
        else
        {
            // Q3: 180-270 degrees (128-192 in our scale)
            dy = -dy;
            if (dx >= dy)
                angle = 128 + (dy * 32) / (dx + 1);
            else
                angle = 192 - (dx * 32) / (dy + 1);
        }
    }

    return (uint8_t)angle;
}
