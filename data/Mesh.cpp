#include "Mesh.hpp"

namespace MinimalCoupler
{
Mesh::Mesh() 
    : _dimensions(0), _meshType(MeshType::PROVIDED)
{}

std::string_view Mesh::getMeshName() const
{
    return _meshName;
}

void Mesh::setMeshName(std::string meshName)
{
    _meshName = std::move(meshName);
}

void Mesh::setMeshVertices(std::vector<Point> vertices)
{
   _vertices = std::move(vertices); 
}

void Mesh::addDataToMesh(const std::string& dataName)
{
    _dataFields[dataName] = std::vector<double>{};
}

int Mesh::getMeshDimensions() const
{
    return _dimensions;
}
        
void Mesh::setMeshDimensions(int dimensions)
{
    _dimensions = dimensions;
}

size_t Mesh::getVertexCount() const
{
    return _vertices.size();
}

void Mesh::allocateDataFields()
{
    size_t dataSize = _vertices.size() * _dimensions;
    for (auto& [name, data] : _dataFields) {
        data.resize(dataSize);
    }
}

const std::vector<Point>& Mesh::getMeshVertices() const
{
    return _vertices;
}

void Mesh::setVertexMapping(std::vector<Point>&& vertexMapping)
{
    _vertexMapping = std::move(vertexMapping);
}

const std::vector<Point>& Mesh::getVertexMapping() const
{
    return _vertexMapping;
}

std::vector<double>& Mesh::getDataField(const std::string& dataName)
{
    return _dataFields.at(dataName);
}

const std::vector<double>& Mesh::getDataField(const std::string& dataName) const
{
    return _dataFields.at(dataName);
}
}