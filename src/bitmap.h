#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include <span>

namespace tt08
{

class Bitmap
{
public:
    Bitmap(unsigned int width, unsigned int height);

    constexpr std::uint8_t& pixel(unsigned int x, unsigned int y) noexcept
    {
        assert(x < m_width);
        assert(y < m_height);
        assert(y * m_width + x < m_pixels.size());
        return m_pixels[y * m_width + x];
    }
    
    constexpr std::uint8_t pixel(unsigned int x, unsigned int y) const noexcept
    {
        assert(x < m_width);
        assert(y < m_height);
        assert(y * m_width + x < m_pixels.size());
        return m_pixels[y * m_width + x];
    }
    
    constexpr std::span<const std::uint8_t> data() const noexcept
    {
        return m_bitmap;
    }
    
    constexpr std::span<const std::uint8_t> pixels() const noexcept
    {
        return m_pixels;
    }

    constexpr unsigned int width() const noexcept
    {
        return m_width;
    }

    constexpr unsigned int height() const noexcept
    {
        return m_height;
    }

private:
    std::vector<std::uint8_t> m_bitmap;
    std::span<std::uint8_t> m_pixels;
    unsigned int m_width;
    unsigned int m_height;
};

} // namespace tt08