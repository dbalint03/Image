#pragma once

#include <string>
#include <vector>

class Image
{
private:
    bool is_png(const std::pair<
                std::vector<uint8_t>::const_iterator,
                std::vector<uint8_t>::const_iterator> &signature) const;
    size_t no_of_channels;
    std::uint8_t bit_depth;
    std::uint8_t color_type;
    std::uint8_t interlace_method;

    std::vector<uint8_t> parse_png(std::vector<std::uint8_t> &file_data);
    std::vector<uint8_t> decompress_data(std::vector<uint8_t> &compressed_data);
    void reconstruct_pixels(const std::vector<uint8_t> &decompressed_data);
    size_t get_no_of_channels(std::uint8_t color_type) const;

public:
    std::uint32_t width, height;
    std::vector<uint8_t> pixels;

    Image(std::string fileName);
    Image(std::vector<std::uint8_t> file_data);

    uint8_t &pixel(int x, int y, int channel);
    void print_pixel(int x, int y);
    size_t get_row_size();
};