#include "Utils.hpp"

namespace MinimalCoupler::Utils
{

std::vector<double> vectorDifference(std::vector<double>& first, std::vector<double>& second)
{
    std::vector<double> diff(first.size());
    for(size_t i = 0; i < first.size(); i++)
        diff[i] = first[i] - second[i];

    return diff;
}

double computeVectorNorm(const std::vector<double>& data)
{
    double squaredSum = 0.0;
    for (auto it: data)
    {
        squaredSum += it * it;
    }

    return std::sqrt(squaredSum);
}

double euclideanDistance(const Point &p1, const Point &p2)
{
    return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}


void sendVector(int socket, const std::vector<double> &data)
{
    size_t size = data.size();
    send(socket, &size, sizeof(size), 0);
    send(socket, data.data(), size * sizeof(double), 0);
}
void recvVector(int socket, std::vector<double> &data)
{
    size_t size;
    recv(socket, &size, sizeof(size), 0);
    data.resize(size);
    recv(socket, data.data(), size * sizeof(double), MSG_WAITALL);
}
void sendBool(int socket, bool value)
{
    send(socket, &value, sizeof(value), 0);
}
void recvBool(int socket, bool &value)
{
    recv(socket, &value, sizeof(value), MSG_WAITALL);
}

void sendPoints(int socket, const std::vector<Point> &points)
{
    size_t size = points.size();
    send(socket, &size, sizeof(size), 0);
    send(socket, points.data(), size * sizeof(Point), 0);
}
void recvPoints(int socket, std::vector<Point> &points)
{
    size_t size;
    recv(socket, &size, sizeof(size), 0);
    points.resize(size);
    recv(socket, points.data(), size * sizeof(Point), MSG_WAITALL);
}
}