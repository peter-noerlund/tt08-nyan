#include "bitmap.h"

#include <array>
#include <cstdint>

namespace tt08
{

Bitmap::Bitmap(unsigned int width, unsigned int height)
    : m_width(width)
    , m_height(height)
{
    struct UnalignedUint32
    {
        std::uint16_t low = 0;
        std::uint16_t high = 0;

        UnalignedUint32& operator=(std::uint32_t value)
        {
            low = value & 0xFFFF;
            high = value >> 16;
            return *this;
        }
    };

    struct BitmapFileHeader
    {
        std::array<char, 2> type;
        UnalignedUint32 size;
        UnalignedUint32 reserved;
        UnalignedUint32 bitsOffset;
    };

    static_assert(sizeof(BitmapFileHeader) == 14);

    struct BitmapInfoHeader
    {
        std::uint32_t size;
        std::int32_t width;
        std::int32_t height;
        std::uint16_t planes;
        std::uint16_t bitCount;
        std::uint32_t compression;
        std::uint32_t imageSize;
        std::int32_t xPelsPerMeter;
        std::int32_t yPelsPerMeter;
        std::uint32_t colorUsed;
        std::uint32_t colorsImportant;
    };

    static_assert(sizeof(BitmapInfoHeader) == 40);

    struct Rgb
    {
        std::uint8_t blue = 0;
        std::uint8_t green = 0;
        std::uint8_t red = 0;
        std::uint8_t reserved = 0;
    };

    constexpr unsigned int colorCount = 64;
    constexpr unsigned int colorTableSize = colorCount * sizeof(Rgb);
    m_bitmap.resize(sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + colorTableSize + width * height);

    auto fileHeader = reinterpret_cast<BitmapFileHeader*>(m_bitmap.data());
    fileHeader->type = {'B', 'M'};
    fileHeader->size = m_bitmap.size();
    fileHeader->reserved = 0;
    fileHeader->bitsOffset = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + colorTableSize;

    auto infoHeader = reinterpret_cast<BitmapInfoHeader*>(m_bitmap.data() + sizeof(BitmapFileHeader));
    infoHeader->size = sizeof(BitmapInfoHeader);
    infoHeader->width = width;
    infoHeader->height = 0 - height;
    infoHeader->planes = 1;
    infoHeader->bitCount = 8;
    infoHeader->compression = 0;
    infoHeader->imageSize = width * height;
    infoHeader->xPelsPerMeter = 2835;
    infoHeader->yPelsPerMeter = 2835;
    infoHeader->colorUsed = colorCount;
    infoHeader->colorsImportant = colorCount;

    std::span<Rgb, colorCount> colorTable(reinterpret_cast<Rgb*>(m_bitmap.data() + sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader)), colorCount);

    for (unsigned int i = 0; i != colorCount; ++i)
    {
        // Encode as 00BBGGRR
        auto red = (i & 0x03) << 6;
        auto green = ((i >> 2) & 0x03) << 6;
        auto blue = ((i >> 4) & 0x03) << 6;

        colorTable[i].red = red;
        colorTable[i].green = green;
        colorTable[i].blue = blue;
    }

    m_pixels = {m_bitmap.data() + sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + colorTableSize, m_width * m_height};
}

} // namespace tt08