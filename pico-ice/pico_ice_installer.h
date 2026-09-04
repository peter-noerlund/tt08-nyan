#pragma once

#include "unique_udev.h"
#include "unique_udev_device.h"

#include <filesystem>
#include <string>

namespace tt09_levenshtein
{

class PicoIceInstaller
{
public:
    PicoIceInstaller();

    void setTtyPath(const std::filesystem::path& path);
    void setStoragePath(const std::filesystem::path& path);
    void install(const std::filesystem::path& filename, bool persistent);

private:
    UniqueUdevDevice findPicoIce();
    std::filesystem::path findTty(udev_device* parentDevice);
    std::filesystem::path findStorage(udev_device* parentDevice);
    UniqueUdevDevice findChildDevice(udev_device* parentDevice, const std::string& subsystem);
    void execute(std::string_view script);

    UniqueUdev m_udev;
    std::filesystem::path m_ttyPath;
    std::filesystem::path m_storagePath;
};

} // namespace tt09_levenshtein