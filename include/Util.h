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
