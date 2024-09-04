#include <array>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
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

static void analyze(std::span<const std::uint8_t> pixels, unsigned int width, unsigned int height, unsigned int widthInBytes)
{
    std::vector<std::uint8_t> pmodPixels;
    pmodPixels.resize(width * height);

    for (unsigned int y = 0; y != height; ++y)
    {
        for (unsigned int x = 0; x != width; ++x)
        {
            auto red = pixels[(height - y - 1) * widthInBytes + x * 3] / 85;
            auto green = pixels[(height - y - 1) * widthInBytes + x * 3 + 1] / 85;
            auto blue = pixels[(height - y - 1) * widthInBytes + x * 3 + 1] / 85;

            pmodPixels.at(y * width + x) = (red << 4) | (green << 2) | blue;
        }
    }

    constexpr unsigned int strideX = 1;
    constexpr unsigned int strideY = 1;

    std::map<std::array<std::uint8_t, strideX * strideY>, unsigned int> mapping;
    for (unsigned int y = 0; y < height; y += strideY)
    {
        for (unsigned int x = 0; x < width; x += strideX)
        {
            std::array<std::uint8_t, strideX * strideY> values;
            for (unsigned int j = 0; j != strideY; ++j)
            {
                for (unsigned int i = 0; i != strideX; ++i)
                {
                    values[j * strideX + i] = pmodPixels.at((y + j) * width + x + i);
                }
            }
            auto res = mapping.insert({values, 1});
            if (!res.second)
            {
                res.first->second++;
            }
        }
    }

}

static constexpr std::uint8_t rgb222(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
{
    return (r << 4) | (g << 2) | b;
}

static void bmp2verilog(const char* name, const char* filename)
{
    static const std::array<std::uint8_t, 15> palette = {
        rgb222(0,0,0),
        rgb222(0,1,3),
        rgb222(0,2,3),
        rgb222(0,3,0),
        rgb222(2,2,2),
        rgb222(2,2,3),
        rgb222(3,0,0),
        rgb222(3,1,0),
        rgb222(3,1,2),
        rgb222(3,2,0),
        rgb222(3,2,2),
        rgb222(3,2,3),
        rgb222(3,3,0),
        rgb222(3,3,2),
        rgb222(3,3,3)
    };

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

    unsigned int widthInBytes = infoHeader.width * 3;
    if (widthInBytes % 4 != 0)
    {
        widthInBytes += 4 - widthInBytes % 4;
    }

    file.seekg(fileHeader.bitsOffset, std::ifstream::beg);

    std::vector<std::uint8_t> pixels;
    pixels.resize(widthInBytes * infoHeader.height);
    file.read(reinterpret_cast<char*>(pixels.data()), pixels.size());
    file.close();

    //analyze(pixels, infoHeader.width, infoHeader.height, widthInBytes);

    std::map<std::uint8_t, unsigned int> colors;

    std::cout << "reg [3:0] " << name << " [" << (infoHeader.width - 1) << ":0][" << (infoHeader.height - 1) << ":0];\n";
    std::cout << "initial begin\n";

    for (std::int32_t y = 0; y != infoHeader.height; ++y)
    {
        for (std::int32_t x = 0; x != infoHeader.width; ++x)
        {
            auto b = pixels.at((infoHeader.height - y - 1) * widthInBytes + x * 3) / 85;
            auto g = pixels.at((infoHeader.height - y - 1) * widthInBytes + x * 3 + 1) / 85;
            auto r = pixels.at((infoHeader.height - y - 1) * widthInBytes + x * 3 + 2) / 85;

            std::uint8_t color = (r << 4) | (g << 2) | b;

            /*
            auto res = colors.insert({color, 1});
            if (!res.second)
            {
                res.first->second++;
            }
            */

            auto it = std::lower_bound(palette.begin(), palette.end(), color);
            if (it == palette.end() || *it != color)
            {
                std::string string = "Invalid color.";
                string += " searched " + std::to_string(color) + " but got " + std::to_string(*it);
                throw std::invalid_argument(string.c_str());
            }

            std::cout << "    " << name << "[" << x << "][" << y << "] = 4'd" << (it - palette.begin()) << ";\n";
        }
    }

    /*
    std::cerr << colors.size() << std::endl;

    for (auto [color, count] : colors)
    {
        auto r = color >> 4;
        auto g = (color >> 2) & 0x03;
        auto b = color & 0x03;
        std::cout << "rgb222(" << static_cast<unsigned int>(r) << ',' << static_cast<unsigned int>(g) << ',' << static_cast<unsigned int>(b) << ')' << std::endl;
    }
    */

    std::cout << "end" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: bmp2verilog NAME FILE" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        bmp2verilog(argv[1], argv[2]);
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Caught exception: " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}