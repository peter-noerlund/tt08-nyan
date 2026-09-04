#pragma once

#include <libudev.h>

#include <memory>

namespace tt09_levenshtein
{

struct UdevEnumerateFreer
{
    inline void operator()(udev_enumerate* enumerate) const noexcept
    {
        udev_enumerate_unref(enumerate);
    }
};

using UniqueUdevEnumerate = std::unique_ptr<udev_enumerate, UdevEnumerateFreer>;

} // namespace tt09_levenshtein