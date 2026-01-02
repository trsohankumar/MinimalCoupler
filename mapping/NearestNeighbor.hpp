#pragma once
#include <vector>
#include <memory>
#include "../data/Point.hpp"

namespace MinimalCoupler
{
    struct KDNode
    {
        Point point;
        size_t nodeID;
        std::unique_ptr<KDNode> left;
        std::unique_ptr<KDNode> right;
        int axis;
    };
    class NearestNeighbor
    {
        // Construct KD tree from the points that I have received
        // Then for a set of points received from the other mesh, implement a query mechanism to find the nearest neighbor in my set of points
    public:
        std::vector<Point> computeNearestNeighbors(const std::vector<Point>& sourcePointsconst, const std::vector<Point> &queryPoints);

    private:
        std::unique_ptr<KDNode> buildKDTree(const std::vector<Point> &points, int axis);
        
        void findNearestNeighbor(const Point& queryPoint, const KDNode* node, Point& bestPoint, double& bestDist) const;

        double euclideanDistance(const Point& p1, const Point& p2) const;
        std::unique_ptr<KDNode> _root;
    };
}