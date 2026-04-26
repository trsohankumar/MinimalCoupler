#pragma once
#include "constants.hpp"
#include "precice/types.hpp"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace MinimalCoupler {
enum class MeshType { PROVIDED, RECEIVED };

/**
 * @brief The structure represents a point 
 * 
 * This point is exchanged during coupling. 
 * The vertices of the mesh are made up of points which are of this struct type
 */
struct Point {
    int    id {};
    double x {};
    double y {};
};

/**
 * @brief The class represents a coupling mesh abstraction
 * 
 * Seperates the mesh abstaraction from the participant so 
 * that the mulitple data vectors can be tracked without the
 * participant needing to maintain all this information 
 * itself.
 */
class Mesh {
public:
    Mesh();

    precice::string_view getMeshName() const;

    void setMeshName(std::string meshName);

    void setMeshVertices(std::vector<Point> vertices);

    void setVertexMapping(std::vector<Point>&& vertexMapping);

    void addDataToMesh(const std::string& dataName, int timeWindow, std::vector<double>&& inputData = {});

    int getMeshDimensions() const;

    void setMeshDimensions(int dimensions);

    size_t getVertexCount() const;

    const std::vector<Point>& getMeshVertices() const;
    const std::vector<Point>& getVertexMapping() const;

    std::vector<double>& getDataField(const std::string& dataName, int timeWindow);

    bool checkIfDataFieldExists(const std::string& dataName) const;

    bool checkIfTimeWindowExists(const std::string& dataName, int timeWindow) const;

    std::vector<int> getAvailableTimeWindows(const std::string& dataName) const;

    bool checkIfVertexIdExists(const int vertexId) const;

    bool requiresInitialData() const;

    void getDataForVertexId(
        precice::string_view                   dataName,
        precice::span<const precice::VertexID> vertexId,
        precice::span<double>                  values,
        int                                    timeWindow
    ) const;

private:
    std::string                                                         _meshName;
    std::vector<Point>                                                  _vertices;
    std::vector<Point>                                                  _vertexMapping;
    int                                                                 _dimensions;
    std::unordered_map<std::string, std::map<int, std::vector<double>>> _dataFields;
    MeshType                                                            _meshType;
    bool _requiresInitialData = Constants::REQUIRES_INITIAL_DATA;
};

} // namespace MinimalCoupler