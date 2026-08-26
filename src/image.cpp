#include "image.hpp"
#include "bytereader.hpp"
#include "paeth.hpp"

#include <zlib.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <assert.h>
#include <algorithm>
#include <cmath>

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
    std::vector<uint8_t> compressed_data;
    while (!done)
    {
        auto length = reader.read_u32_be();
        std::cout << " Length: " << length << '\n';
        auto type = reader.read_u32_be();
        std::cout << "Chunk: " << type << std::endl;
        auto data = reader.read_bytes(length);
        reader.read_u32_be();
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
            compressed_data.reserve((Image::get_row_size() + 1) * Image::height);
            break;
        case idat:
            compressed_data.insert(compressed_data.end(), data.first, data.second);
            break;
        };
    }
    auto decompressed_data = decompress_data(compressed_data);
    reconstruct_pixels(decompressed_data);
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

std::vector<uint8_t> Image::decompress_data(std::vector<uint8_t> &compressed_data)
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
        throw std::runtime_error("inflateInit failed");
    }

    std::cout << "result: " << result << std::endl;

    uint8_t buffer[4096];
    std::vector<uint8_t> decompressed_data;
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
        decompressed_data.insert(
            decompressed_data.end(),
            buffer,
            buffer + produced);
    } while (result != Z_STREAM_END);

    inflateEnd(&stream);
    return decompressed_data;
}

int Image::reconstruct_pixels(const std::vector<uint8_t> &decompressed_data)
{
    Image::no_of_channels = get_no_of_channels(Image::color_type);

    const size_t row_bytes = Image::get_row_size();
    const size_t row_stride = row_bytes + 1;
    const size_t bytes_per_pixel = Image::no_of_channels;

    pixels.resize(Image::height * row_bytes);

    for (size_t y = 0; y < Image::height; y++)
    {
        const size_t row_offset = y * row_stride;
        const std::uint8_t filter_method = decompressed_data[row_offset];
        const size_t output_offset = y * row_bytes;
        switch (filter_method)
        {
        case 0: // None
            std::copy_n(
                decompressed_data.data() + row_offset + 1,
                row_bytes,
                pixels.data() + output_offset);
            break;
        case 1: // Sub
        {
            for (size_t x = 0; x < row_bytes; ++x)
            {
                std::uint8_t byte = decompressed_data[row_offset + 1 + x];
                if (x < bytes_per_pixel)
                {
                    pixels[output_offset + x] = byte + 0;
                }
                else
                {
                    std::uint8_t left = pixels[output_offset + x - bytes_per_pixel];
                    pixels[output_offset + x] = byte + left;
                }
            }
            break;
        }
        case 2: // Up
        {
            for (size_t x = 0; x < row_bytes; ++x)
            {
                std::uint8_t byte = decompressed_data[row_offset + 1 + x];
                std::uint8_t above = 0 == y ? 0 : pixels[output_offset - row_bytes + x];
                pixels[output_offset + x] = byte + above;
            }
            break;
        }
        case 3: // Average
        {
            for (size_t x = 0; x < row_bytes; ++x)
            {
                std::uint8_t byte = decompressed_data[row_offset + 1 + x];
                std::uint8_t above = 0 == y ? 0 : pixels[output_offset - row_bytes + x];
                std::uint8_t left = x < bytes_per_pixel ? 0 : pixels[output_offset + x - bytes_per_pixel];

                pixels[output_offset + x] = byte + std::floor((above + left) / 2);
            }
            break;
        }
        case 4: // Paeth
        {
            for (size_t x = 0; x < row_bytes; ++x)
            {
                std::uint8_t byte = decompressed_data[row_offset + 1 + x];
                std::uint8_t above = 0 == y ? 0 : pixels[output_offset - row_bytes + x];
                std::uint8_t left = x < bytes_per_pixel ? 0 : pixels[output_offset + x - bytes_per_pixel];
                std::uint8_t upper_left = 0 == y || x < bytes_per_pixel ? 0 : pixels[output_offset - row_bytes + x - bytes_per_pixel];
                pixels[output_offset + x] = byte + paeth_predictor(left, above, upper_left);
            }
            break;
        }
        default:
            break;
        }
    }

    return 0;
}

size_t Image::get_no_of_channels(std::uint8_t color_type) const
{
    if (bit_depth != 8)
        throw std::runtime_error("Only 8-bit PNGs are supported");

    if (interlace_method != 0)
        throw std::runtime_error("Interlaced PNGs are not supported");

    switch (color_type)
    {
    case 0: // Grayscale
        return 1;
        break;
    case 2: // Truecolor
        return 3;
        break;
    case 3: // Indexed
        return 1;
        break;
    case 4: // Grayscale and alpha
        return 2;
        break;
    case 6: // Truecolor and alpha
        return 4;
        break;
    default:
        throw std::runtime_error("Unsupported color type");
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

size_t Image::get_row_size()
{
    return (Image::width * Image::no_of_channels * Image::bit_depth + 7) / 8;
}