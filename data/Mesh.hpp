#pragma once
#include "Point.hpp"
#include "precice/types.hpp"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace MinimalCoupler
{
enum class MeshType
{
    PROVIDED,
    RECEIVED
};

class Mesh
{
  public:
    Mesh();

    std::string_view getMeshName() const;

    void setMeshName(std::string meshName);

    void setMeshVertices(std::vector<Point> vertices);

    void setReadMapping(std::vector<Point> &&vertexMapping);

    void setWriteMapping(std::vector<Point> &&vertexMapping);

    void addDataToMesh(const std::string &dataName, int timeWindow, std::vector<double> &&inputData = {});

    int getMeshDimensions() const;

    void setMeshDimensions(int dimensions);

    size_t getVertexCount() const;

    void allocateDataFields();

    const std::vector<Point> &getMeshVertices() const;

    const std::vector<Point> &getReadMapping() const;

    const std::vector<Point> &getWriteMapping() const;

    std::vector<double> &getDataField(const std::string &dataName, int timeWindow);

    bool checkIfDataFieldExists(const std::string &dataName) const;

    bool checkIfTimeWindowExists(const std::string &dataName, int timeWindow) const;

    std::vector<int> getAvailableTimeWindows(const std::string &dataName) const;

    bool checkIfVertexIdExists(const int vertexId) const;

    void getDataForVertexId(precice::string_view dataName, precice::span<const precice::VertexID> vertexId,
                            precice::span<double> values, int timeWindow);

  private:
    std::string _meshName;
    std::vector<Point> _vertices;
    std::vector<Point> _readVertexMapping;
    std::vector<Point> _writeVertexMapping;
    int _dimensions;
    std::unordered_map<std::string, std::map<int, std::vector<double>>> _dataFields;
    MeshType _meshType;
};

} // namespace MinimalCoupler