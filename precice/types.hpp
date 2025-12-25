#pragma once
#include <span>
#include <string_view>

namespace precice
{
    template<typename T>
    using span = std::span<T>;
    using VertexID = int;
    using string_view = std::string_view;
}