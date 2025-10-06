// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "SystemInfo.hpp"
#include <iostream>
#include <string>
#include <fstream>

#if defined(_WIN32)
#include <comdef.h>
#include <versionhelpers.h>
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <cstdlib>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/utsname.h>
#elif defined(__linux__)
#include <cstdlib>
#endif

std::string GetOS()
{
#if defined(_WIN32)
    // Basic Windows version (reliable only with manifesting)
    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (GetVersionExW((LPOSVERSIONINFOW)&osvi))
    {
        std::wstring wversion = L"Windows " + std::to_wstring(osvi.dwMajorVersion) + L"." +
                                std::to_wstring(osvi.dwMinorVersion);
        return std::string(wversion.begin(), wversion.end());
    }
    return "Windows (unknown version)";

#elif defined(__APPLE__)
    char buf[128] = {};
    std::string product;
    FILE* pipe = popen("sw_vers -productName", "r");
    if (pipe && fgets(buf, sizeof(buf), pipe))
    {
        product = buf;
    }
    pclose(pipe);

    pipe = popen("sw_vers -productVersion", "r");
    if (pipe && fgets(buf, sizeof(buf), pipe))
    {
        std::string version = buf;
        product += version;
    }
    pclose(pipe);

    // Trim newline
    if (!product.empty() && product.back() == '\n')
    {
        product.pop_back();
    }

    return product.empty() ? "macOS (unknown)" : product;

#elif defined(__linux__)
    std::ifstream os_release("/etc/os-release");
    std::string line, name, version;
    while (std::getline(os_release, line))
    {
        if (line.rfind("PRETTY_NAME=", 0) == 0)
        {
            size_t start = line.find("\"");
            size_t end = line.rfind("\"");
            if (start != std::string::npos && end > start)
            {
                return line.substr(start + 1, end - start - 1);
            }
        }
    }
    return "Linux (unknown distro)";

#else
    return "Unknown OS";
#endif
}

std::string GetCPU()
{
#if defined(_WIN32)
    HKEY hKey;
    char cpuName[0x40];
    DWORD size = sizeof(cpuName);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)cpuName,
                             &size) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return std::string(cpuName);
        }
        RegCloseKey(hKey);
    }
    return "Unknown CPU";

#elif defined(__APPLE__)
    char cpuModel[256];
    size_t size = sizeof(cpuModel);
    if (sysctlbyname("machdep.cpu.brand_string", cpuModel, &size, nullptr, 0) == 0)
    {
        return std::string(cpuModel);
    }
    return "Unknown CPU";

#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line))
    {
        if (line.find("model name") != std::string::npos)
        {
            auto pos = line.find(':');
            if (pos != std::string::npos) return line.substr(pos + 2);
        }
    }
    return "Unknown CPU";

#else
    return "Unsupported platform";
#endif
}

long GetRAMMegabytes()
{
#if defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status))
    {
        return static_cast<long>(status.ullTotalPhys / (1024 * 1024));
    }
    return -1;

#elif defined(__APPLE__)
    int64_t mem_bytes;
    size_t len = sizeof(mem_bytes);
    if (sysctlbyname("hw.memsize", &mem_bytes, &len, NULL, 0) == 0)
    {
        return static_cast<long>(mem_bytes / (1024 * 1024));
    }
    return -1;

#elif defined(__linux__)
    std::ifstream meminfo("/proc/meminfo");
    std::string label;
    long mem_kb;
    while (meminfo >> label >> mem_kb)
    {
        if (label == "MemTotal:")
        {
            return mem_kb / 1024;
        }
    }
    return -1;

#else
    return -1;
#endif
}

std::vector<std::string> GetGPUs()
{
    std::vector<std::string> gpus;

#if defined(_WIN32)
    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(dd);

    for (size_t i = 0; EnumDisplayDevicesA(0, i, &dd, 0); ++i)
    {
        gpus.push_back(dd.DeviceString);
    }

#elif defined(__APPLE__)
    CFMutableDictionaryRef match = IOServiceMatching("IOPCIDevice");
    io_iterator_t iter;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, match, &iter) == KERN_SUCCESS)
    {
        io_object_t device;
        while ((device = IOIteratorNext(iter)))
        {
            CFTypeRef model =
                IORegistryEntryCreateCFProperty(device, CFSTR("model"), kCFAllocatorDefault, 0);
            if (model && CFGetTypeID(model) == CFDataGetTypeID())
            {
                CFDataRef data = (CFDataRef)model;
                std::string name((const char*)CFDataGetBytePtr(data), CFDataGetLength(data));
                gpus.push_back(name);
                CFRelease(model);
            }
            IOObjectRelease(device);
        }
        IOObjectRelease(iter);
    }

#elif defined(__linux__)
    FILE* pipe = popen("lspci | grep -iE \"vga|3d controller\"", "r");
    if (!pipe)
    {
        std::cerr << "Failed to run the GPU searching command\n";
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe))
    {
        gpus.push_back(buffer);
    }
    pclose(pipe);

#else
    gpus.push_back("[Unsupported platform]");
#endif

    if (gpus.empty()) gpus.push_back("[No GPU found]");
    return gpus;
}

std::vector<std::string> GetAllMountPoints()
{
    std::vector<std::string> mounts;

#if defined(_WIN32)
    DWORD driveMask = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter)
    {
        if (driveMask & (1 << (letter - 'A')))
        {
            std::string drive = std::string(1, letter) + ":\\";
            mounts.push_back(drive);
        }
    }

#elif defined(__linux__) || defined(__APPLE__)
    std::ifstream mountsFile("/proc/mounts");
    std::string device, mountpoint;
    while (mountsFile >> device >> mountpoint)
    {
        mounts.push_back(mountpoint);
        std::string restOfLine;
        std::getline(mountsFile, restOfLine);
    }
#endif

    return mounts;
}

std::filesystem::space_info GetDiskInfo(const std::string& path)
{
    try
    {
        return std::filesystem::space(path);
    }
    catch (const std::exception&)
    {
    }
    std::filesystem::space_info emptySpaceInfo;
    return emptySpaceInfo;
}
