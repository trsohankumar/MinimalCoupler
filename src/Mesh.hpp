#pragma once
#include "Point.hpp"
#include "constants.hpp"
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

    precice::string_view getMeshName() const;

    void setMeshName(std::string meshName);

    void setMeshVertices(std::vector<Point> vertices);

    void setVertexMapping(std::vector<Point> &&vertexMapping);

    void addDataToMesh(const std::string &dataName, int timeWindow, std::vector<double> &&inputData = {});

    int getMeshDimensions() const;

    void setMeshDimensions(int dimensions);

    size_t getVertexCount() const;

    void allocateDataFields();

    const std::vector<Point> &getMeshVertices() const;
    const std::vector<Point> &getVertexMapping() const;

    std::vector<double> &getDataField(const std::string &dataName, int timeWindow);

    bool checkIfDataFieldExists(const std::string &dataName) const;

    bool checkIfTimeWindowExists(const std::string &dataName, int timeWindow) const;

    std::vector<int> getAvailableTimeWindows(const std::string &dataName) const;

    bool checkIfVertexIdExists(const int vertexId) const;

    bool requiresInitialData() const;

    void getDataForVertexId(precice::string_view dataName, precice::span<const precice::VertexID> vertexId, precice::span<double> values, int timeWindow) const;

  private:
    std::string _meshName;
    std::vector<Point> _vertices;
    std::vector<Point> _vertexMapping;
    int _dimensions;
    std::unordered_map<std::string, std::map<int, std::vector<double>>> _dataFields;
    MeshType _meshType;
    bool _requiresInitialData = Constants::REQUIRES_INITIAL_DATA;
};

} // namespace MinimalCoupler