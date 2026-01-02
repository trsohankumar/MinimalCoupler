#include "NearestNeighbor.hpp"
#include <algorithm>
#include <limits>
#include <cmath>

namespace MinimalCoupler
{
NearestNeighbor::NearestNeighbor()
    : _root(nullptr)
{}

std::unique_ptr<KDNode> NearestNeighbor::buildKDTree(const std::vector<Point>& points, int axis)
{
    auto root = std::make_unique<KDNode>();

    std::nth_element(points.begin(), points.begin() + points.size() /2, points.end(), [&axis](const Point& lhs, const Point&rhs){
        if (axis == 0)
            return lhs.x < rhs.x;
        else
            return lhs.y < rhs.y;
    });
    size_t id = points.size() / 2;
    root->point = points[id];
    root->nodeID = id;
    root->axis = axis;
    root->left = buildKDTree(std::vector<Point>(points.begin(), points.begin() + points.size() / 2), axis == 0 ? 1 : 0);
    root->right = buildKDTree(std::vector<Point>(points.begin() + points.size() / 2 + 1, points.end()), axis == 0 ? 1 : 0);

    return root;
}

double NearestNeighbor::euclideanDistance(const Point& p1, const Point& p2) const
{
    return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

void NearestNeighbor::findNearestNeighbor(const Point& queryPoint, const KDNode* node, Point& bestPoint, double& bestDist) const
{
    if (!node)
        return;

    double dist = euclideanDistance(queryPoint, node->point);

    if (dist < bestDist)
    {
        bestDist = dist;
        bestPoint = node->point;
    }

    int axis = node->axis;
    KDNode* nextNode = nullptr;
    KDNode* otherNode = nullptr;
    if ((axis == 0 && queryPoint.x < node->point.x) || (axis == 1 && queryPoint.y < node->point.y))
    {
        nextNode = node->left.get();
        otherNode = node->right.get();
    }
    else
    {
        nextNode = node->right.get();
        otherNode = node->left.get();
    }
    findNearestNeighbor(queryPoint, nextNode, bestPoint, bestDist);
    double axisDist = (axis == 0) ? std::abs(queryPoint.x - node->point.x) : std::abs(queryPoint.y - node->point.y);
    if (axisDist < bestDist)
    {
        findNearestNeighbor(queryPoint, otherNode, bestPoint, bestDist);
    }
}

std::vector<Point> NearestNeighbor::computeNearestNeighbors(const std::vector<Point>& sourcePoints, const std::vector<Point>& queryPoints)
{
    _root = buildKDTree(sourcePoints, 0);
    std::vector<Point> nearestNeighbors(queryPoints.size());

    for (size_t i = 0; i < queryPoints.size(); ++i)
    {
        Point bestPoint;
        double bestDist = std::numeric_limits<double>::max();
        findNearestNeighbor(queryPoints[i], _root.get(), bestPoint, bestDist);
        nearestNeighbors[i] = bestPoint;
    }

    return nearestNeighbors;
}
}