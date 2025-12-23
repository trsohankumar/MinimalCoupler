#include "Mesh.hpp"

namespace MinimalCoupler
{
Mesh::Mesh(std::string & meshName)
    : _meshName(std::move(meshName))
{}
}