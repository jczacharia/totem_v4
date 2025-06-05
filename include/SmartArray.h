#pragma once

template <typename T, size_t W, size_t H>
class SmartArray
{
  protected:
    std::array<T, W * H + 1> data_{};

    FORCE_INLINE_ATTR size_t xy(const size_t x, const size_t y)
    {
        if (x >= W || y >= H)
        {
            return 0;
        }

        return y * W + x + 1;
    }

  public:
    SmartArray() = default;
    ~SmartArray() = default;

    SmartArray(const SmartArray &other)
        : data_(other.data_)
    {
    }

    SmartArray(SmartArray &&other) noexcept
        : data_(std::move(other.data_))
    {
    }

    SmartArray &operator=(const SmartArray &other)
    {
        if (this != &other)
        {
            data_ = other.data_;
        }

        return *this;
    }

    SmartArray &operator=(SmartArray &&other) noexcept
    {
        if (this != &other)
        {
            data_ = std::move(other.data_);
        }

        return *this;
    }

    T &operator()(const size_t x, const size_t y)
    {
        return data_[xy(x, y)];
    }

    // T operator()(const size_t x, const size_t y) const
    // {
    //     return data_[xy(x, y)];
    // }

    void fill(const T &fillVal)
    {
        data_.fill(fillVal);
    }

    void all(std::function<void(T &)> func)
    {
        for (auto &val : data_)
        {
            func(val);
        }
    }

    void clear()
    {
        data_ = {};
    }

    T *data()
    {
        return data_.data();
    }
};
