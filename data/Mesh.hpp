#pragma once
#include<string>
#include<vector>
#include<unordered_map>
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
        void setVertexMapping(std::vector<Point>&& vertexMapping);
        void addDataToMesh(const std::string& dataName);
        int getMeshDimensions() const;
        void setMeshDimensions(int dimensions);
        size_t getVertexCount() const;
        void allocateDataFields();
        const std::vector<Point>& getMeshVertices() const;
        const std::vector<Point>& getVertexMapping() const;
        std::vector<double>& getDataField(const std::string& dataName);
        const std::vector<double>& getDataField(const std::string& dataName) const;

    private:
        std::string _meshName;
        std::vector<Point> _vertices;
        std::vector<Point> _vertexMapping;
        int _dimensions;
        std::unordered_map<std::string, std::vector<double>> _dataFields;
        MeshType _meshType;
    };

}