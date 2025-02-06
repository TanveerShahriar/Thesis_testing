#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int main()
{
    std::string path = "./Input";

    try {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                std::cout << "Found: " << entry.path() << std::endl;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    return 0;
}
