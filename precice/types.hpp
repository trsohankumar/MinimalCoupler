#pragma once
#include <span>
#include <string_view>

namespace precice
{
    template <typename T, std::size_t Extent = std::dynamic_extent>
    class span : public std::span<T, Extent>
    {
    public:
        using std::span<T, Extent>::span;

        operator std::string_view() const
            requires std::same_as<T, const char>
        {
            return std::string_view(this->data(), this->size());
        }

        bool operator==(const char *str) const
            requires std::same_as<T, const char>
        {
            return std::string_view(this->data(), this->size()) == str;
        }
    };
    using VertexID = int;
    using string_view = span<const char>;
}