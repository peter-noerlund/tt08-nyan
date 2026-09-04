#pragma once

#include <libudev.h>

#include <memory>

namespace tt09_levenshtein
{

struct UdevFreer
{
    inline void operator()(udev* udev) const noexcept
    {
        udev_unref(udev);
    }
};

using UniqueUdev = std::unique_ptr<udev, UdevFreer>;

} // namespace tt09_levenshtein