#pragma once

#include<string>
#include<vector>
#include "precice/types.hpp"

namespace MinimalCoupler
{
using VertexID = int;

class ParticipantImplementation
{
public:

    ParticipantImplementation(precice::string_view participantName, precice::string_view configurationFileName,int solverProcessIndex, int solverProcessSize);

    virtual ~ParticipantImplementation() = default;

    // Mesh methods
    virtual int getMeshDimensions(precice::string_view meshName) const = 0;

    virtual void setMeshVertices(
      precice::string_view meshName,
      precice::span< const double > coordinates, ::precice::span< VertexID > ids) = 0;

    // Data exchange methods
    virtual void readData(
      const std::string& meshName,
      const std::string& dataName,
      const std::vector<int>& vertexIDs,
      double relativeReadTime,
      std::vector<double>& values) const = 0;

    virtual void writeData(
      const std::string& meshName,
      const std::string& dataName,
      const std::vector<int>& vertexIDs,
      const std::vector<double>& values) = 0;

    // Steering methods
    virtual void initialize() = 0;
    virtual void advance(double computedTimeStepSize) = 0;
    virtual void finalize() = 0;

    // Status queries
    virtual bool isCouplingOngoing() const = 0;
    virtual bool requiresInitialData() const = 0;
    virtual bool requiresWritingCheckpoint() const = 0;
    virtual bool requiresReadingCheckpoint() const = 0;

    virtual double getMaxTimeStepSize() const = 0;

    // Profiling
    virtual void startProfilingSection(const std::string& name) = 0;
    virtual void stopLastProfilingSection() = 0;

protected:
    std::string _participantName;
    std::string _remoteParticipantName;
    std::string _configFileName;
    int _rank;
    int _size;

};
}