#include <array>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <fstream>
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

static void bmp2verilog(const char* name, const char* filename)
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

    std::cout << "reg [" << (infoHeader.width * 6 - 1) << " : 0] " << name << " [" << (infoHeader.height - 1) << " : 0];\n";
    std::cout << "initial begin\n";

    for (std::int32_t y = 0; y != infoHeader.height; ++y)
    {
        std::cout << "    " << name << "[" << (infoHeader.height - y - 1) << "] = " << (infoHeader.width * 6) << "'b";
        for (std::int32_t x = 0; x != infoHeader.width; ++x)
        {
            for (int c = 0; c != 3; ++c)
            {
                static const std::array<std::string_view, 4> bits = {"00", "01", "10", "11"};
                auto val = pixels.at(y * widthInBytes + (infoHeader.width - x - 1) * 3 + c) / 85;

                std::cout << bits.at(val);
            }
        }
        std::cout << ";\n";
    }

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