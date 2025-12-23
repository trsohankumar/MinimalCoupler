#pragma once
#include<string>
#include<vector>
#include<unordered_map>
#include "Point.hpp"

namespace MinimalCoupler
{
    enum MeshType
    {
        PROVIDED,
        RECEIVED
    };

    class Mesh
    {
    public:
        const std::string getMeshName() const;
        void setMeshName(std::string meshName);
        void setMeshVertices(std::vector<Point>& vertices);
        void addDataToMesh(const std::string& dataName);

    private:
        std::string _meshName;
        std::vector<Point> _vertices;
        std::unordered_map<std::string, std::vector<double>> _dataVectors;
        MeshType _meshType;
    };

}