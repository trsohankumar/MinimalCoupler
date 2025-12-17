#pragma once
#include<string>
#include<vector>
#include<unordered_map>

namespace MinimalCoupler
{
    class Point
    {
    public:
    private:
        double x, y, z;
    };
    class Mesh
    {
    public:
    private:
        std::string _meshName;
        std::vector<Point> _points;
        std::unordered_map<std::string, std::vector<double>> _dataVectors;
    };

}