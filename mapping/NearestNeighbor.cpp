#include "NearestNeighbor.hpp"
#include <algorithm>
#include <limits>
#include <cmath>

namespace MinimalCoupler
{
std::unique_ptr<KDNode> NearestNeighbor::buildKDTree(const std::vector<Point>& points, int axis)
{
    // Base case: empty vector
    if (points.empty())
        return nullptr;

    auto root = std::make_unique<KDNode>();

    // Create a mutable copy since nth_element needs to reorder elements
    std::vector<Point> mutablePoints = points;

    std::nth_element(mutablePoints.begin(), mutablePoints.begin() + mutablePoints.size() /2, mutablePoints.end(), [&axis](const Point& lhs, const Point&rhs){
        if (axis == 0)
            return lhs.x < rhs.x;
        else
            return lhs.y < rhs.y;
    });
    size_t id = mutablePoints.size() / 2;
    root->point = mutablePoints[id];
    root->axis = axis;
    root->left = buildKDTree(std::vector<Point>(mutablePoints.begin(), mutablePoints.begin() + mutablePoints.size() / 2), axis == 0 ? 1 : 0);
    root->right = buildKDTree(std::vector<Point>(mutablePoints.begin() + mutablePoints.size() / 2 + 1, mutablePoints.end()), axis == 0 ? 1 : 0);

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

void NearestNeighbor::mapConsistent(const std::vector<Point>& mapping, const std::vector<double>& sourceData, std::vector<double>& targetData, int dimensions)
{
    targetData.resize(mapping.size() * dimensions);

    for (size_t i = 0; i < mapping.size(); ++i)
    {
        int sourceVertexID = mapping[i].id;
        for (int d = 0; d < dimensions; ++d)
        {
            targetData[i * dimensions + d] = sourceData[sourceVertexID * dimensions + d];
        }
    }
}

void NearestNeighbor::mapConservative(const std::vector<Point>& mapping, const std::vector<double>& sourceData,               std::vector<double>& targetData, int dimensions)
{
    std::fill(targetData.begin(), targetData.end(), 0.0);

    for (size_t i = 0; i < mapping.size(); ++i)
    {
        int targetVertexID = mapping[i].id;
        for (int d = 0; d < dimensions; ++d)
        {
            targetData[targetVertexID * dimensions + d] += sourceData[i * dimensions + d];
        }
    }
}
}