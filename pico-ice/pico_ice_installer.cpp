#include "pico_ice_installer.h"

#include "from_hex.h"
#include "unique_enumerate_udev.h"
#include "unique_libmnt_table.h"

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/read.hpp>
#include <asio/read_until.hpp>
#include <asio/serial_port.hpp>
#include <asio/write.hpp>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <libudev.h>
#include <libmount.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <iterator>
#include <stdexcept>

namespace tt09_levenshtein
{

PicoIceInstaller::PicoIceInstaller()
    : m_udev(udev_new())
{
}

void PicoIceInstaller::setTtyPath(const std::filesystem::path& path)
{
    m_ttyPath = path;
}

void PicoIceInstaller::setStoragePath(const std::filesystem::path& path)
{
    m_storagePath = path;
}

void PicoIceInstaller::install(const std::filesystem::path& filename, bool persistent)
{
    if (m_ttyPath.empty() || m_storagePath.empty())
    {
        auto picoIceDevice = findPicoIce();
        if (m_ttyPath.empty())
        {
            m_ttyPath = findTty(picoIceDevice.get());
        }
        if (m_storagePath.empty())
        {
            m_storagePath = findStorage(picoIceDevice.get());
        }
    }

    auto targetPath = m_storagePath / filename.filename();

    fmt::println("Copying {} to {}", filename.string(), targetPath.string());
    std::filesystem::copy_file(filename, targetPath, std::filesystem::copy_options::overwrite_existing);

    ::sync();

    std::string script = fmt::format(
        "import ice\n"
        "from machine import Pin\n"
        "fpga = ice.fpga(cdone=Pin(26), clock=Pin(24), creset=Pin(27), cram_cs=Pin(9), cram_mosi=Pin(8), cram_sck=Pin(10), frequency=48)\n"
        "fpga.stop()\n"
        "file = open(\"{}\", \"br\")\n",
        filename.filename().string());

    if (persistent)
    {
        script += 
            "flash = ice.flash(miso=Pin(8), mosi=Pin(11), sck=Pin(10), cs=Pin(9))\n"
            "flash.write(file)\n"
            "fpga.start()\n";
    }
    else
    {
        script += 
            "fpga.start()\n"
            "fpga.cram(file)\n";
    }

    fmt::println("Running MicroPython script:\n\033[33m{}\033[0m", script);


    execute(script);
}

UniqueUdevDevice PicoIceInstaller::findPicoIce()
{
    UniqueUdevEnumerate udevEnum(udev_enumerate_new(m_udev.get()));

    udev_enumerate_add_match_subsystem(udevEnum.get(), "usb");
    udev_enumerate_scan_devices(udevEnum.get());

    auto listEntries = udev_enumerate_get_list_entry(udevEnum.get());
    decltype(listEntries) listEntry;

    UniqueUdevDevice picoIceDevice;

    udev_list_entry_foreach(listEntry, listEntries)
    {
        UniqueUdevDevice usbDevice(udev_device_new_from_syspath(m_udev.get(), udev_list_entry_get_name(listEntry)));
        if (!usbDevice)
        {
            continue;
        }

        auto devType = udev_device_get_devtype(usbDevice.get());
        if (devType != std::string_view("usb_device"))
        {
            continue;
        }

        if (fromHex<unsigned int>(udev_device_get_sysattr_value(usbDevice.get(), "idVendor")) != 0x1209)
        {
            continue;
        }
        
        if (fromHex<unsigned int>(udev_device_get_sysattr_value(usbDevice.get(), "idProduct")) != 0xB1C0)
        {
            continue;
        }

        return usbDevice;
    }

    throw std::runtime_error("Unable to find PICO-ICE device");
}

UniqueUdevDevice PicoIceInstaller::findChildDevice(udev_device* parentDevice, const std::string& subsystem)
{
    UniqueUdevEnumerate udevEnum(udev_enumerate_new(m_udev.get()));
    udev_enumerate_add_match_parent(udevEnum.get(), parentDevice);
    udev_enumerate_add_match_subsystem(udevEnum.get(), subsystem.c_str());
    udev_enumerate_scan_devices(udevEnum.get());

    auto listEntries = udev_enumerate_get_list_entry(udevEnum.get());
    if (!listEntries)
    {
        throw std::runtime_error(fmt::format("Found no child device in subsystem {}", subsystem));
    }

    UniqueUdevDevice device(udev_device_new_from_syspath(m_udev.get(), udev_list_entry_get_name(listEntries)));

    return device;
}

std::filesystem::path PicoIceInstaller::findTty(udev_device* parentDevice)
{
    auto ttyDevice = findChildDevice(parentDevice, "tty");
    return udev_device_get_devnode(ttyDevice.get());
}

std::filesystem::path PicoIceInstaller::findStorage(udev_device* parentDevice)
{
    auto storageDevice = findChildDevice(parentDevice, "block");
    auto devicePath = udev_device_get_devnode(storageDevice.get());

    UniqueLibmntTable table(mnt_new_table());
    if (mnt_table_parse_mtab(table.get(), nullptr) != 0)
    {
        throw std::runtime_error("Error parsing mount table");
    }

    auto fs = mnt_table_find_source(table.get(), devicePath, MNT_ITER_FORWARD);
    if (!fs)
    {
        throw std::runtime_error(fmt::format("Unable to found mount point for device: {}", devicePath));
    }

    return mnt_fs_get_target(fs);
}

void PicoIceInstaller::execute(std::string_view script)
{
    asio::io_context context;
    asio::serial_port device(context, m_ttyPath.string());

    // Soft reset by sending Ctrl+C, Ctrl+C, Ctrl+B, Ctrl+D
    const auto resetSequence = std::to_array<const std::uint8_t>({0x03, 0x03, 0x02, 0x04});
    asio::write(device, asio::buffer(resetSequence));

    std::string lineBuffer;
    asio::read_until(device, asio::dynamic_buffer(lineBuffer), ">>>");

    // Enter raw REPL by sending Ctrl+A
    const auto rawReplCommand = std::to_array<const std::uint8_t>({0x01});
    asio::write(device, asio::buffer(rawReplCommand));

    lineBuffer.clear();
    asio::read_until(device, asio::dynamic_buffer(lineBuffer), "raw REPL; CTRL-B to exit\r\n>");

    // Send script followed by Ctrl+D
    asio::write(device, asio::buffer(script));

    const auto commitCommand = std::to_array<const std::uint8_t>({0x04});
    asio::write(device, asio::buffer(commitCommand));

    // Read OK response
    std::array<char, 2> okBuffer;
    asio::read(device, asio::buffer(okBuffer));

    if (okBuffer[0] != 'O' || okBuffer[1] != 'K')
    {
        throw std::runtime_error("Device did not return expected response");
    }

    // Read response
    std::string response;
    auto cnt = asio::read_until(device, asio::dynamic_buffer(response), "\x04>");
    response.resize(cnt);

    std::string_view responseView(response);

    auto exceptionStart = responseView.find('\x04');
    if (exceptionStart == std::string_view::npos)
    {
        throw std::runtime_error("Device returned invalid data");
    }
    
    auto exceptionEnd = responseView.find('\x04', exceptionStart + 1);
    if (exceptionEnd == std::string_view::npos)
    {
        throw std::runtime_error("Device returned invalid data");
    }

    auto exception = responseView.substr(exceptionStart + 1, exceptionEnd - exceptionStart - 1);
    if (!exception.empty())
    {
        throw std::runtime_error(fmt::format("Device threw exception: {}", exception));
    }

    // Finalize by sending Ctrl+C, Ctrl+C, Ctrl+B
    const auto finalCommand = std::to_array<const std::uint8_t>({0x03, 0x03, 0x02});
    asio::write(device, asio::buffer(finalCommand));
}

} // namespace tt09_levenshtein