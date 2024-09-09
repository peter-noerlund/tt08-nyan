#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <span>
#include <stdexcept>
#include <vector>

struct BitmapFileHeader
{
    std::uint32_t size;
    std::uint32_t reserved;
    std::uint32_t bitsOffset;
};

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

class Bitmap
{
public:
    explicit Bitmap(const char* filename)
        : m_filename(filename)
    {
        std::ifstream file;
        file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
        file.open(filename, std::ifstream::in | std::ifstream::binary);

        std::array<char, 2> signature;
        file.read(signature.data(), signature.size());
        if (signature[0] != 'B' || signature[1] != 'M')
        {
            throw std::invalid_argument("Invalid bitmap");
        }

        BitmapFileHeader fileHeader;
        file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));

        BitmapInfoHeader infoHeader;
        file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

        if (infoHeader.bitCount != 24)
        {
            throw std::invalid_argument("Only 24-bit bitmaps supported");
        }
        if (infoHeader.compression != 0)
        {
            throw std::invalid_argument("Compressed bitmaps are not supported");
        }
        if (infoHeader.height < 0)
        {
            throw std::invalid_argument("Negative height not supported");
        }

        m_width = infoHeader.width;
        m_height = infoHeader.height;
        auto widthInBytes = infoHeader.width * 3;
        if (widthInBytes % 4 != 0)
        {
            widthInBytes += 4 - widthInBytes % 4;
        }

        std::vector<std::uint8_t> line;
        line.resize(widthInBytes);

        m_pixels.resize(m_width * m_height);

        file.seekg(fileHeader.bitsOffset, std::ifstream::beg);

        for (unsigned int y = 0; y != m_height; ++y)
        {
            file.read(reinterpret_cast<char*>(line.data()), line.size());

            for (unsigned int x = 0; x != m_width; ++x)
            {
                m_pixels[(m_height - y - 1) * m_width + x] = toRgb222(line[x * 3 + 2], line[x * 3 + 1], line[x * 3]);
            }
        }
    }

    constexpr unsigned int width() const noexcept
    {
        return m_width;
    }

    constexpr unsigned int height() const noexcept
    {
        return m_height;
    }

    constexpr std::span<const std::uint8_t> pixels() const noexcept
    {
        return m_pixels;
    }

    constexpr std::uint8_t pixel(unsigned int x, unsigned int y) const
    {
        if (x >= m_width || y >= m_height)
        {
            throw std::invalid_argument("Out of range");
        }
        return m_pixels[y * m_width + x];
    }

    constexpr const std::filesystem::path& filename() const noexcept
    {
        return m_filename;
    }

private:
    constexpr std::uint8_t toRgb222(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
    {
        return ((r / 85) << 4) | ((g / 85) << 2) | (b / 85);
    }

    unsigned int m_width = 0;
    unsigned int m_height = 0;
    std::filesystem::path m_filename;
    std::vector<std::uint8_t> m_pixels;
};

class BitmapMaker
{
public:
    using PixelSet = std::array<std::uint8_t, 4>;

    void analyze(const Bitmap& bitmap)
    {
        if (bitmap.width() % 2 != 0)
        {
            throw std::invalid_argument("Bitmap with must be an even number");
        }

        for (unsigned int y = 0; y != bitmap.height(); ++y)
        {
            PixelSet pixelSet;
            for (unsigned int x = 0; x != bitmap.width(); x += pixelSet.size())
            {
                for (std::size_t i = 0; i != pixelSet.size(); ++i)
                {
                    pixelSet[i] = bitmap.pixel(x + i, y);
                }

                auto res = m_pixelSetHistogram.insert({pixelSet, 1});
                if (!res.second)
                {
                    res.first->second++;
                }
            }
        }
    }

    void createPalette()
    {
        if (m_pixelSetHistogram.size() > 64)
        {
            throw std::runtime_error("Got more than 32 pixel pairs");
        }

        std::vector<std::pair<PixelSet, unsigned int>> sortedPixelSets;
        sortedPixelSets.assign(m_pixelSetHistogram.begin(), m_pixelSetHistogram.end());
        std::sort(sortedPixelSets.begin(), sortedPixelSets.end(), [](const auto& lhs, const auto& rhs)
        {
            return rhs.second < lhs.second;
        });

        for (std::size_t i = 0; i != sortedPixelSets.size(); ++i)
        {
            m_palette[sortedPixelSets[i].first] = static_cast<std::uint8_t>(i);
        }
    }

    void writePalette(const std::filesystem::path& outputDir) const
    {
        auto filename = outputDir / "palette.svh";

        std::ofstream file;
        file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        file.open(filename.string(), std::ofstream::out | std::ofstream::trunc);

        file << "reg [5:0] palette[" << (m_palette.size() - 1) << ":0][" << (m_palette.begin()->first.size() - 1) << ":0];\n";
        file << "initial begin\n";

        std::vector<PixelSet> palette;
        palette.resize(m_palette.size());
        for (auto [pixelSet, encoding] : m_palette)
        {
            palette.at(encoding) = pixelSet;
        }

        static const std::array<std::string_view, 4> mapping({"00", "01", "10", "11"});
        for (std::size_t i = 0; i != palette.size(); ++i)
        {
            auto pixelSet = palette[i];
            for (std::size_t j = 0; j != pixelSet.size(); ++j)
            {
                auto pixel = pixelSet[j];

                file << "    palette[" << i << "][" << j << "] = 6'b" << mapping.at(pixel >> 4) << mapping.at((pixel >> 2) & 0x03) << mapping.at(pixel & 0x03) << ";\n";
            }
        }

        file << "end\n" << std::flush;
    }

    void writeBitmap(const Bitmap& bitmap, const std::filesystem::path& outputDir) const
    {
        auto filename = outputDir / bitmap.filename().filename().replace_extension("svh");

        std::ofstream file;
        file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        file.open(filename.string(), std::ofstream::out | std::ofstream::trunc);

        auto name = bitmap.filename().filename().replace_extension("").string();

        PixelSet pixelSet;

        file << "reg [5:0] " << name << "[" << (bitmap.width() / pixelSet.size() - 1) << ":0][" << (bitmap.height() - 1) << ":0];\n";
        file << "initial begin\n";

        for (unsigned int y = 0; y != bitmap.height(); ++y)
        {
            for (unsigned int x = 0; x != bitmap.width(); x += pixelSet.size())
            {
                for (std::size_t j = 0; j != pixelSet.size(); ++j)
                {
                    pixelSet[j] = bitmap.pixel(x + j, y);
                }

                file << "    " << name << "[" << (x / pixelSet.size()) << "][" << y << "] = 6'd" << m_palette.at(pixelSet) << ";\n";
            }
        }

        file << "end\n" << std::flush;
    }

private:
    std::map<PixelSet, unsigned int> m_pixelSetHistogram;
    std::map<PixelSet, unsigned int> m_palette;
};

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::cerr << "Usage: make_bitmaps OUTPUT_DIR FILE [FILE...]" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        std::vector<Bitmap> bitmaps;
        for (int i = 2; i != argc; ++i)
        {
            bitmaps.emplace_back(argv[i]);
        }

        std::filesystem::path outputDir = argv[1];

        BitmapMaker bitmapMaker;
        for (const auto& bitmap : bitmaps)
        {
            bitmapMaker.analyze(bitmap);
        }
        bitmapMaker.createPalette();
        bitmapMaker.writePalette(outputDir);
        for (const auto& bitmap : bitmaps)
        {
            bitmapMaker.writeBitmap(bitmap, outputDir);
        }
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Caught exception: " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
}