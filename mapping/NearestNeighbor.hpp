#pragma once
#include <vector>
#include <memory>
#include "../data/Point.hpp"

namespace MinimalCoupler
{
    class NearestNeighbor
    {
     // Construct KD tree from the points that I have received
     // Then for a set of points received from the other mesh, implement a query mechanism to find the nearest neighbor in my set of points
    public:
        void build(const std::vector<Point>& points);
        std::vector<Point> computeNearestNeighbors(const std::vector<Point>& queryPoints) const;
    private: 
        struct KDNode
        {
            Point point;
            std::unique_ptr<KDNode> left;
            std::unique_ptr<KDNode> right;
            int axis;
        };

        std::unique_ptr<KDNode> root;
    };
}