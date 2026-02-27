#pragma once

#include<vector>
#include<math.h>
#include <iostream>
#include <sys/socket.h>

#include "Point.hpp"

namespace MinimalCoupler::Utils
{

// Math utility functions
double computeVectorNorm(const std::vector<double>& data);
std::vector<double> vectorDifference(std::vector<double>& first, std::vector<double>& second);
double euclideanDistance(const Point &p1, const Point &p2);

// Socket send and recv functions
void sendVector(int socket, const std::vector<double> &data);
void recvVector(int socket, std::vector<double> &data);
void sendBool(int socket, bool value);
void recvBool(int socket, bool &value);
void sendPoints(int socket, const std::vector<Point> &points);
void recvPoints(int socket, std::vector<Point> &points);
}