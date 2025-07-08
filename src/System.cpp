#include "System.hpp"
#include <cstdlib>

void OpenInFileManager(const std::string& path)
{
#ifdef _WIN32
    std::string command = "explorer \"" + path + "\"";
    (void)system(command.c_str());
#elif __APPLE__
    std::string command = "open \"" + path + "\"";
    (void)system(command.c_str());
#elif __linux__
    std::string command = "xdg-open \"" + path + "\"";
    (void)system(command.c_str());
#else
#error Platform not supported
#endif
}
