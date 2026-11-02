#include "file.h"
#include <fstream>
#include <stdexcept>

namespace hookhound::io {
    std::vector<std::uint8_t> read_file(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path); // TODO: failed to open file is vague
        }
        std::int64_t file_size = file.tellg(); 
        std::vector<std::uint8_t> data(file_size); 
        file.seekg(0);
        file.read(reinterpret_cast<char*>(data.data()), file_size);
        return data;
    }
}