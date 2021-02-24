#include <bitset>
#include <iostream>

int main(int argc, char *argv[]) {
    uint8_t v = 34;
    std::cout << std::bitset<8>(v) << std::endl;
    std::cout << (uint32_t)(v >> 4) << std::endl;
    std::cout << (uint32_t)(v & 0x0F) << std::endl;
    std::cout << std::bitset<8>((uint32_t)(v >> 4)) << std::endl;
    std::cout << std::bitset<8>((uint32_t)(v & 0x0F)) << std::endl;
    std::cout << std::bitset<8>(0x0F) << std::endl;

    return 0;
}
