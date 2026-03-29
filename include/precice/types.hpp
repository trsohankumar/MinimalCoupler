#pragma once

#include <ostream>
#include <span>
#include <string_view>

namespace precice
{
template <typename T, std::size_t Extent = std::dynamic_extent> class span : public std::span<T, Extent>
{
  public:
    using std::span<T, Extent>::span;

    explicit operator std::string_view() const
        requires std::same_as<T, const char>
    {
        return std::string_view(this->data(), this->size());
    }

    bool operator==(const char *str) const
        requires std::same_as<T, const char>
    {
        return std::string_view(this->data(), this->size()) == str;
    }

    explicit operator std::string() const
        requires std::same_as<T, const char>
    {
        return std::string(this->data(), this->size());
    }

    friend std::ostream &operator<<(std::ostream &os, const span &s)
        requires std::same_as<T, const char>
    {
        return os << std::string_view(s.data(), s.size());
    }
};
using VertexID = int;
using string_view = span<const char>;
} // namespace precice