#pragma once

#include "libmount.h"

#include <memory>

namespace tt09_levenshtein
{

struct LibmntTableFreer
{
    inline void operator()(libmnt_table* table) const noexcept
    {
        mnt_unref_table(table);
    }
};

using UniqueLibmntTable = std::unique_ptr<libmnt_table, LibmntTableFreer>;

} // namespace tt09_levenshtein