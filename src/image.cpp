#include "image.hpp"
#include "bytereader.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <zlib.h>
#include <assert.h>
#include <chrono>

void print_hex(const ByteReader::ByteRange hex);

Image::Image(std::string fileName)
{
    constexpr std::uint32_t iend = 0x49454E44; // "IEND" in big-endian order
    constexpr std::uint32_t ihdr = 0x49484452; // "IHDR" in big-endian order
    constexpr std::uint32_t idat = 0x49444154; // "IDAT" in big-endian order

    std::ifstream inputFile(fileName, std::ios::binary);
    std::vector<unsigned char> fileData;
    for (char byte; inputFile.get(byte);)
    {
        fileData.push_back(static_cast<unsigned char>(byte));
    }

    ByteReader reader(fileData);
    auto signature = reader.read_bytes(8);
    print_hex(signature);
    if (is_png(signature))
    {
        std::cout << "PNG!" << std::endl;
    }
    bool done = false;
    while (!done)
    {
        auto length = reader.read_u32_be();
        std::cout << " Length: " << length << '\n';
        auto type = reader.read_u32_be();
        std::cout << "Chunk: " << type << std::endl;
        auto data = reader.read_bytes(length);
        auto crc = reader.read_u32_be();
        switch (type)
        {
        case iend:
            std::cout << "end of file" << std::endl;
            done = true;
            break;
        case ihdr:
            Image::width = ByteReader::to_u32_be(ByteReader::ByteRange(data.first, data.first + 4));
            Image::height = ByteReader::to_u32_be(ByteReader::ByteRange(data.first + 4, data.first + 8));
            Image::bit_depth = ByteReader::to_u8_be(ByteReader::ByteRange(data.first + 8, data.first + 9));
            Image::color_type = ByteReader::to_u8_be(ByteReader::ByteRange(data.first + 9, data.first + 10));
            Image::compression_method = ByteReader::to_u8_be(ByteReader::ByteRange(data.first + 10, data.first + 11));
            Image::filter_method = ByteReader::to_u8_be(ByteReader::ByteRange(data.first + 11, data.first + 12));
            Image::interlace_method = ByteReader::to_u8_be(ByteReader::ByteRange(data.first + 12, data.first + 13));
            assert(0 == Image::compression_method);
            assert(0 == Image::filter_method);
            break;
        case idat:
            compressed_data.insert(compressed_data.end(), data.first, data.second);
            break;
        };
    }
    decompress_data();
    reconstruct_pixels();
}

bool Image::is_png(const ByteReader::ByteRange &signature) const
{
    const uint8_t png_signature[] = {
        0x89, 0x50, 0x4E, 0x47,
        0x0D, 0x0A, 0x1A, 0x0A};
    return std::equal(
        signature.first,
        signature.second,
        std::begin(png_signature));
}

void print_hex(const ByteReader::ByteRange hex)
{
    for (auto it = hex.first; it != hex.second; ++it)
    {
        std::cout << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << static_cast<int>(*it)
                  << ' ';
    }
    std::cout << std::dec << '\n';
}

void Image::decompress_data()
{
    std::cout << "Decompressing data..." << std::endl;
    z_stream stream{};
    stream.next_in = compressed_data.data();
    stream.avail_in = compressed_data.size();

    std::cout << "result: " << std::endl;
    int result = inflateInit(&stream);
    if (result != Z_OK)
    {
        printf("Error: inflateInit %d\n", result);
        return;
    }

    std::cout << "result: " << result << std::endl;

    uint8_t buffer[4096];
    do
    {
        stream.next_out = buffer;
        stream.avail_out = sizeof(buffer);
        result = inflate(&stream, Z_NO_FLUSH);

        if (result != Z_OK &&
            result != Z_STREAM_END)
        {
            inflateEnd(&stream);
            throw std::runtime_error("inflate failed");
        }

        std::size_t produced = sizeof(buffer) - stream.avail_out;
        Image::decompressed_data.insert(
            decompressed_data.end(),
            buffer,
            buffer + produced);
    } while (result != Z_STREAM_END);

    std::cout << "result: " << result << std::endl;
}

int Image::reconstruct_pixels()
{
    std::chrono::steady_clock::time_point begin2 = std::chrono::steady_clock::now();
    std::cout << "reconstructing pixels..." << std::endl;
    ByteReader reader(Image::decompressed_data);
    std::uint8_t filter = reader.read_u8_be();
    assert(filter == 0);
    Image::no_of_channels = get_channel_number(Image::color_type);
    pixels.resize(Image::width * Image::height * Image::no_of_channels);
    std::cout << "here " << std::endl;
    // std::cout << std::hex
    //           << std::setw(2)
    //           << std::setfill('0');
    std::chrono::steady_clock::time_point end2 = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - begin2).count() << "[ms]" << std::endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (size_t y = 0; y < Image::height; y++)
    {
        for (size_t x = 0; x < Image::width; x++)
        {
            ByteReader::ByteRange bytes = reader.read_bytes(no_of_channels);
            // Image::pixels[(y * width + x) * Image::no_of_channels + 0] = ByteReader::to_u8_be(ByteReader::ByteRange(bytes.first, bytes.first + 1));
            // Image::pixels[(y * width + x) * Image::no_of_channels + 1] = ByteReader::to_u8_be(ByteReader::ByteRange(bytes.first + 1, bytes.first + 2));
            // Image::pixels[(y * width + x) * Image::no_of_channels + 2] = ByteReader::to_u8_be(ByteReader::ByteRange(bytes.first + 2, bytes.first + 3));
            for (size_t c = 0; c < Image::no_of_channels; c++)
            {
                // uint8_t p = reader.read_u8_be();
                // std::cout << "p: " << static_cast<int>(p) << std::endl;
                Image::pixels[(y * width + x) * Image::no_of_channels + c] = ByteReader::to_u8_be(ByteReader::ByteRange(bytes.first + c, bytes.first + c + 1));
                // Image::pixels[(y * width + x) * Image::no_of_channels + c] = p;
            }
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    std::cout << std::dec << '\n';
    return 0;
}

size_t Image::get_channel_number(std::uint8_t color_type) const
{
    switch (color_type)
    {
    case 0:
        return 1;
        break;
    case 2:
        return 3;
        break;
    case 3:
        return 1;
        break;
    case 4:
        return 2;
        break;
    case 6:
        return 4;
        break;
    }
}

uint8_t &Image::pixel(int x, int y, int channel)
{
    return pixels[(y * width + x) * Image::no_of_channels + channel];
}

void Image::print_data() const
{
    std::cout << "width: " << Image::width << std::endl;
    std::cout << "height: " << Image::height << std::endl;
    std::cout << "bit depth: " << static_cast<int>(Image::bit_depth) << std::endl;
    std::cout << "color type: " << static_cast<int>(Image::color_type) << std::endl;
    std::cout << "compression method: " << static_cast<int>(Image::compression_method) << std::endl;
    std::cout << "filter method: " << static_cast<int>(Image::filter_method) << std::endl;
    std::cout << "interlace method: " << static_cast<int>(Image::interlace_method) << std::endl;
    std::cout << "compressed data size: " << Image::compressed_data.size() << std::endl;
    std::cout << "decompressed data size: " << Image::decompressed_data.size() << std::endl;
}

void Image::print_pixel(int x, int y)
{
    std::cout << std::hex
              << std::setw(2)
              << std::setfill('0');
    std ::cout << static_cast<int>(Image::pixel(x, y, 0))
               << " ,"
               << static_cast<int>(Image::pixel(x, y, 1))
               << " ,"
               << static_cast<int>(Image::pixel(x, y, 2))
               << " ,"
               << static_cast<int>(Image::pixel(x, y, 3))
               << " ";
}