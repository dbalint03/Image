#include "bytereader.hpp"

#include <gtest/gtest.h>

TEST(ByteReaderTest, ReadBytes)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x7, 0x89, 0x00, 0xFF};
    ByteReader reader(data);

    EXPECT_FALSE(reader.eof());
    auto bytes = reader.read_bytes(4);
    EXPECT_EQ(bytes.first, data.begin());
    EXPECT_EQ(bytes.second, data.begin() + 4);
    EXPECT_EQ(reader.get_pos(), 4);
    EXPECT_FALSE(reader.eof());
}

TEST(ByteReaderTest, ReadBigEndianUint32)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x78};
    ByteReader reader(data);

    EXPECT_EQ(reader.read_u32_be(), 0x12345678u);
    EXPECT_EQ(reader.get_pos(), data.size());
    EXPECT_TRUE(reader.eof());
}

TEST(ByteReaderTest, ReadBigEndianUint16)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x78};
    ByteReader reader(data);

    EXPECT_EQ(reader.read_u16_be(), 0x1234u);
    EXPECT_EQ(reader.read_u16_be(), 0x5678u);
    EXPECT_TRUE(reader.eof());
}

TEST(ByteReaderTest, ReadBigEndianUint8)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x78};
    ByteReader reader(data);

    EXPECT_EQ(reader.read_u8_be(), 0x12u);
    EXPECT_EQ(reader.read_u8_be(), 0x34u);
    EXPECT_EQ(reader.read_u8_be(), 0x56u);
    EXPECT_EQ(reader.read_u8_be(), 0x78u);
    EXPECT_TRUE(reader.eof());
}

TEST(ByteReaderTest, EmptyDataThrowsException)
{
    std::vector<std::uint8_t> data{};
    ByteReader reader(data);

    EXPECT_TRUE(reader.eof());
    EXPECT_THROW(reader.read_u32_be(), std::runtime_error);
    EXPECT_THROW(reader.read_u16_be(), std::runtime_error);
    EXPECT_THROW(reader.read_u8_be(), std::runtime_error);
    EXPECT_THROW(reader.read_bytes(1), std::runtime_error);
}

TEST(ByteReaderTest, ReadBytesBeyondEndThrowsException)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x78};
    ByteReader reader(data);

    EXPECT_THROW(reader.read_bytes(5), std::runtime_error);
    EXPECT_EQ(reader.read_u32_be(), 0x12345678u);
    EXPECT_EQ(reader.get_pos(), data.size());
    EXPECT_THROW(reader.read_bytes(1), std::runtime_error);
    EXPECT_EQ(reader.get_pos(), data.size());
}

TEST(ByteReaderTest, ReadBytesInvalidSizeThrowsException)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x78};
    ByteReader reader(data);

    EXPECT_THROW(reader.read_bytes(0), std::runtime_error);
}

TEST(ByteReaderTest, ConvertBigEndianUint32)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x78};
    ByteReader reader(data);

    auto bytes = reader.read_bytes(4);
    EXPECT_EQ(ByteReader::to_u32_be(bytes), 0x12345678u);
}

TEST(ByteReaderTest, ConvertBigEndianUint16)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x78};
    ByteReader reader(data);

    auto bytes = reader.read_bytes(2);
    EXPECT_EQ(ByteReader::to_u16_be(bytes), 0x1234u);
}

TEST(ByteReaderTest, ConvertBigEndianUint8)
{
    std::vector<std::uint8_t> data{0x12, 0x34, 0x56, 0x78};
    ByteReader reader(data);

    auto bytes = reader.read_bytes(1);
    EXPECT_EQ(ByteReader::to_u8_be(bytes), 0x12u);
}