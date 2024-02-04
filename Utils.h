#include <fstream>
#include <vector>
#include <filesystem>

static std::vector<char> ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

static bool FileExists(std::string filePath)
{
    if (std::filesystem::exists(filePath)) 
    {
        return true;
    }
    else 
    {
        return false;
    }
}