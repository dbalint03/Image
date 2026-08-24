add_library(ByteReader)

target_sources(ByteReader
  PRIVATE
    bytereader.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      bytereader.hpp
)
