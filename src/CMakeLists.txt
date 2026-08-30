add_subdirectory(ByteReader)

add_executable(Image
  main.cpp
  image.cpp
)

target_link_libraries(Image
  PRIVATE
    ByteReader
    ZLIB::ZLIB
    SDL3::SDL3
)

add_custom_command(TARGET Image POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_RUNTIME_DLLS:Image>
          $<TARGET_FILE_DIR:Image>
  COMMAND_EXPAND_LISTS
)


target_compile_features(Image PRIVATE cxx_std_17)

target_compile_options(Image PRIVATE -Wall)