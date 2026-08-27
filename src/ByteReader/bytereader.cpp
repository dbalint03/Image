#include "bytereader.hpp"

#include <fstream>

std::uint8_t ByteReader::to_u8_be(ByteRange bytes)
{
    return static_cast<uint32_t>(bytes.first[0]);
}

std::uint16_t ByteReader::to_u16_be(ByteRange bytes)
{
    return (static_cast<uint32_t>(bytes.first[0]) << 8) |
           static_cast<uint32_t>(bytes.first[1]);
}

std::uint32_t ByteReader::to_u32_be(ByteRange bytes)
{
    return (static_cast<uint32_t>(bytes.first[0]) << 24) |
           (static_cast<uint32_t>(bytes.first[1]) << 16) |
           (static_cast<uint32_t>(bytes.first[2]) << 8) |
           static_cast<uint32_t>(bytes.first[3]);
}

ByteReader::ByteRange ByteReader::read_bytes(std::size_t n)
{
    if (n == 0)
        throw std::runtime_error("Cannot read 0 bytes");

    if (n > data.size() - pos)
        throw std::runtime_error("Unexpected end of file");

    auto begin = data.begin() + pos;
    auto end = begin + n;

    pos += n;

    return {begin, end};
}

uint8_t ByteReader::read_u8_be()
{
    auto bytes = read_bytes(1);

    return to_u8_be(bytes);
}

uint16_t ByteReader::read_u16_be()
{
    auto bytes = read_bytes(2);

    return to_u16_be(bytes);
}

uint32_t ByteReader::read_u32_be()
{
    auto bytes = read_bytes(4);

    return to_u32_be(bytes);
}

bool ByteReader::eof() const
{
    return ByteReader::data.size() == pos;
}