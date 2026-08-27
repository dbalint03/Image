#pragma once

#include <vector>
#include <string>

class ByteReader
{
private:
    const std::vector<std::uint8_t> &data;
    std::size_t pos = 0;

public:
    using ByteRange = std::pair<
        std::vector<uint8_t>::const_iterator,
        std::vector<uint8_t>::const_iterator>;

    static std::uint8_t to_u8_be(ByteRange bytes);
    static std::uint16_t to_u16_be(ByteRange bytes);
    static std::uint32_t to_u32_be(ByteRange bytes);

    std::uint8_t read_u8_be();
    std::uint16_t read_u16_be();
    std::uint32_t read_u32_be();
    ByteRange read_bytes(std::size_t n);

    ByteReader(std::vector<std::uint8_t> &_data) : data(_data) {};
    bool eof() const;

    std::size_t get_pos() const
    {
        return pos;
    }
};