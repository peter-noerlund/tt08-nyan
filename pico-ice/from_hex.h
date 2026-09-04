#pragma once

#include <string_view>
#include <stdexcept>

#include <fmt/format.h>

namespace tt09_levenshtein
{

template<typename T>
constexpr T fromHex(std::string_view string)
{
    T value = {};
    while (!string.empty())
    {
        auto c = string.front();
        string.remove_prefix(1);

        value *= 16;
        if (c >= '0' && c <= '9')
        {
            value += static_cast<T>(c - '0');
        }
        else if (c >= 'a' && c <= 'f')
        {
            value += static_cast<T>(c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F')
        {
            value += static_cast<T>(c - 'A' + 10);
        }
        else
        {
            throw std::invalid_argument(fmt::format("Invalid hexadecimal character: {}", c));
        }
    }

    return value;
}

} // namespace tt09_levenshtein