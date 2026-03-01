#include "Mesh.hpp"

namespace MinimalCoupler
{
Mesh::Mesh() : _dimensions(0), _meshType(MeshType::PROVIDED)
{
}

precice::string_view Mesh::getMeshName() const
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

void Mesh::addDataToMesh(const std::string &dataName, int timeWindow, std::vector<double> &&inputData)
{
    if (inputData.empty())
    {
        inputData.resize(_vertices.size() * _dimensions, 0.0);
    }
    _dataFields[dataName][timeWindow] = std::move(inputData);
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
    for (auto &[name, windowMap] : _dataFields)
    {
        for (auto &[window, data] : windowMap)
        {
            data.resize(dataSize, 0.0);
        }
    }
}

const std::vector<Point> &Mesh::getMeshVertices() const
{
    return _vertices;
}

void Mesh::setVertexMapping(std::vector<Point> &&vertexMapping)
{
    _vertexMapping = std::move(vertexMapping);
}

const std::vector<Point> &Mesh::getVertexMapping() const
{
    return _vertexMapping;
}

std::vector<double> &Mesh::getDataField(const std::string &dataName, int timeWindow)
{
    return _dataFields.at(dataName).at(timeWindow);
}

bool Mesh::checkIfDataFieldExists(const std::string &dataName) const
{
    return _dataFields.contains(dataName);
}

bool Mesh::checkIfTimeWindowExists(const std::string &dataName, int timeWindow) const
{
    if (!_dataFields.contains(dataName))
    {
        return false;
    }
    return _dataFields.at(dataName).contains(timeWindow);
}

std::vector<int> Mesh::getAvailableTimeWindows(const std::string &dataName) const
{
    std::vector<int> windows;
    if (_dataFields.contains(dataName))
    {
        for (const auto &[w, data] : _dataFields.at(dataName))
        {
            windows.push_back(w);
        }
    }
    return windows;
}

bool Mesh::checkIfVertexIdExists(const int vertexId) const
{
    return vertexId < _vertices.size();
}

void Mesh::getDataForVertexId(precice::string_view dataName, precice::span<const precice::VertexID> vertexId, precice::span<double> values, int timeWindow) const
{
    const std::vector<double> &dataVector = _dataFields.at(std::string(dataName)).at(timeWindow);

    // store the data for each vertexId in the values array
    size_t outputIdx = 0;
    for (int vid : vertexId)
    {
        size_t startIdx = vid * _dimensions;

        for (int d = 0; d < _dimensions; ++d)
        {
            values[outputIdx] = dataVector[startIdx + d];
            outputIdx++;
        }
    }
}

bool Mesh::requiresInitialData() const
{
    return _requiresInitialData;
}

} // namespace MinimalCoupler