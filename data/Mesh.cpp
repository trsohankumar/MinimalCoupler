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

void Mesh::setReadMapping(std::vector<Point>&& vertexMapping)
{
    _readVertexMapping = std::move(vertexMapping);
}

void Mesh::setWriteMapping(std::vector<Point>&& vertexMapping)
{
    _writeVertexMapping = std::move(vertexMapping);
}

const std::vector<Point>& Mesh::getReadMapping() const
{
    return _readVertexMapping;
}

const std::vector<Point>& Mesh::getWriteMapping() const
{
    return _writeVertexMapping;
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