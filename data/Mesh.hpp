#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "Point.hpp"

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
        void addDataToMesh(const std::string &dataName, double timestamp, std::vector<double> &&inputData = std::vector<double>());
        int getMeshDimensions() const;
        void setMeshDimensions(int dimensions);
        size_t getVertexCount() const;
        void allocateDataFields();
        const std::vector<Point> &getMeshVertices() const;
        const std::vector<Point> &getReadMapping() const;
        const std::vector<Point> &getWriteMapping() const;
        std::vector<double> &getDataField(const std::string &dataName, double timestamp);
        bool checkIfDataFieldExists(const std::string &dataName) const;
        bool checkIfVertexIdExists(const int vertexId) const;

        void getDataForVertexId(const std::string &dataName, const std::vector<int> &vertexId, std::vector<double> &values, double absoluteTime);

    private:
        std::string _meshName;
        std::vector<Point> _vertices;
        std::vector<Point> _readVertexMapping;
        std::vector<Point> _writeVertexMapping;
        int _dimensions;
        std::unordered_map<std::string, std::map<double, std::vector<double>>> _dataFields;
        MeshType _meshType;
    };

}