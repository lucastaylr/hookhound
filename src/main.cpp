#include <fstream>
#include <iostream>
#include <string>
#include <optional>


// open stream to binary file
std::optional<std::ifstream> open_file(const std::string& path) {
    std::ifstream file(path, std::ios:binary);
    if (!file.is_open()) {
        return std::nullopt;
    return file;
}

int main() {
    std::string test_file = "../test/ntdll.dll";
    auto file = open_file(test_file);
    if (!file) {
        std::cout << "File opened successfully: " << test_file << std::endl;
    }
    
}
