#pragma once
#include <memory>
#include <vector>

#include "../data/Point.hpp"

namespace MinimalCoupler
{
struct KDNode
{
    Point point;
    std::unique_ptr<KDNode> left;
    std::unique_ptr<KDNode> right;
    int axis;
};
class NearestNeighbor
{
  public:
    // Construct KD tree from the points that I have received
    // Then for a set of points received from the other mesh, implement a query mechanism to find the nearest neighbor
    // in my set of points
    std::vector<Point> computeNearestNeighbors(const std::vector<Point> &sourcePointsconst,
                                               const std::vector<Point> &queryPoints);

    static void mapConsistent(const std::vector<Point> &mapping, const std::vector<double> &sourceData,
                              std::vector<double> &targetData, int dimensions);

    static void mapConservative(const std::vector<Point> &mapping, const std::vector<double> &sourceData,
                                std::vector<double> &targetData, int dimensions);

  private:
    std::unique_ptr<KDNode> buildKDTree(const std::vector<Point> &points, int axis);

    void findNearestNeighbor(const Point &queryPoint, const KDNode *node, Point &bestPoint, double &bestDist) const;

    double euclideanDistance(const Point &p1, const Point &p2) const;

    std::unique_ptr<KDNode> _root;
};
} // namespace MinimalCoupler