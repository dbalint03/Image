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
    std::uint8_t compression_method;
    std::uint8_t filter_method;
    std::uint8_t interlace_method;
    std::uint32_t width, height;
    std::vector<uint8_t> pixels;
    std::vector<uint8_t> compressed_data;

    int reconstruct_pixels();
    size_t get_channel_number(std::uint8_t color_type) const;
    void decompress_data();

public:
    std::vector<uint8_t> decompressed_data;

    Image(std::string fileName);
    uint8_t &pixel(int x, int y, int channel);
    void print_data() const;
    void print_pixel(int x, int y);
};