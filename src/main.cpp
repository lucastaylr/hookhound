#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include "io/file.h"

int main() {
    std::string test_file = "test/ntdll.dll";
    auto data = hookhound::io::read_file(test_file);
    std::int64_t file_size = data.size();
    std::cout << "File size: " << file_size << std::endl;
}

