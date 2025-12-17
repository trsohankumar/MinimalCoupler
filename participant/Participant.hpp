#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ParticipantImpl.hpp"

namespace MinimalCoupler
{
  class FluidParticipantImplementation;
  class SolidParticipantImplementation;
}

namespace precice
{
class Participant
{
public:
    Participant(
      std::string participantName,
      std::string configurationFileName,
      int         solverProcessIndex,
      int         solverProcessSize);

    ~Participant() = default;

    // Mesh methods
    int getMeshDimensions(const std::string& meshName) const;

    void setMeshVertices(
      const std::string& meshName,
      const std::vector<double>& positions,
      std::vector<int>& ids);

    // Data exchange methods
    void readData(
      const std::string& meshName,
      const std::string& dataName,
      const std::vector<int>& vertexIDs,
      double relativeReadTime,
      std::vector<double>& values) const;

    void writeData(
      const std::string& meshName,
      const std::string& dataName,
      const std::vector<int>& vertexIDs,
      const std::vector<double>& values);

    // Steering methods
    void initialize();
    void advance(double computedTimeStepSize);
    void finalize();

    // Status queries
    bool isCouplingOngoing() const;
    bool requiresInitialData() const;
    bool requiresWritingCheckpoint() const;
    bool requiresReadingCheckpoint() const;

    double getMaxTimeStepSize() const;

    // Profiling
    void startProfilingSection(const std::string& name);
    void stopLastProfilingSection();

private:

    std::unique_ptr<MinimalCoupler::ParticipantImplementation> _impl;
};
}