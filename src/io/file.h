#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace hookhound::io {
    
std::vector<std::uint8_t> read_file(const std::string& path);

}