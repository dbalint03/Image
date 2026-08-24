#include "image.hpp"

#include <iostream>
#include <iomanip>

int main(void)
{
    std::cout << "hello world!" << std::endl;
    Image img("E:\\Projects\\cpp\\Image\\kenny.png");
    img.print_data();
    for (auto it = img.decompressed_data.begin(); it <= img.decompressed_data.begin() + 100; ++it)
    {
        std::cout << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << static_cast<int>(*it)
                  << ' ';
    }
    std::cout << std::dec << '\n';
    img.print_pixel(0,0);
    img.print_pixel(1000,1000);
    img.print_pixel(900,1299);
    // for (auto it = img.pixels.begin(); it <= img.pixels.begin() + 100;)
    // {
    //     std::cout << std::hex
    //               << std::setw(2)
    //               << std::setfill('0')
    //               << static_cast<int>(*it)
    //               << ", "
    //               << static_cast<int>(*++it)
    //               << ", "
    //               << static_cast<int>(*++it)
    //               << ' ';
    // }
    // std::cout << std::dec << '\n';
    return 0;
}