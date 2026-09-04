#include "pico_ice_installer.h"

#include <lyra/lyra.hpp>
#include <fmt/printf.h>

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        std::filesystem::path ttyPath;
        std::filesystem::path storagePath;
        std::filesystem::path filename;
        bool showHelp = false;
        bool persistent = false;
        auto cli = lyra::cli()
            | lyra::opt(ttyPath, "PATH")["-t"]["--tty-path"]("Path to TTY device")
            | lyra::opt(storagePath, "PATH")["-s"]["--storage-path"]("Path to storage device")
            | lyra::opt(persistent)["-p"]["--persistent"]("Install bitstream in flash")
            | lyra::help(showHelp)
            | lyra::arg(filename, "FILENAME").required();
            
        auto result = cli.parse({argc, argv});
        if (!result)
        {
            fmt::println(stderr, "Error parsing command line arguments: {}", result.message());
            return EXIT_FAILURE;
        }
        if (showHelp)
        {
            std::cout << cli << std::endl;
            return EXIT_SUCCESS;
        }

        tt09_levenshtein::PicoIceInstaller installer;
        if (!ttyPath.empty())
        {
            installer.setTtyPath(ttyPath);
        }
        if (!storagePath.empty())
        {
            installer.setStoragePath(storagePath);
        }
        installer.install(filename, persistent);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        fmt::println(stderr, "Caught exception: {}", exception.what());
        return EXIT_FAILURE;
    }
}