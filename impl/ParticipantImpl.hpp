#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "couplingscheme/coupling.hpp"
#include "data/Mesh.hpp"
#include "precice/types.hpp"

namespace MinimalCoupler
{

class ParticipantImplementation
{
  public:
    ParticipantImplementation(precice::string_view participantName, precice::string_view configurationFileName,
                              int solverProcessIndex, int solverProcessSize);

    ~ParticipantImplementation();

    // Mesh methods
    int getMeshDimensions(precice::string_view meshName) const;

    void setMeshVertices(precice::string_view meshName, precice::span<const double> coordinates,
                         precice::span<int> ids);

    // Data exchange methods
    void readData(precice::string_view meshName, precice::string_view dataName,
                  precice::span<const precice::VertexID> vertexIDs, double relativeReadTime,
                  precice::span<double> values) const;

    void writeData(precice::string_view meshName, precice::string_view dataName,
                   precice::span<const precice::VertexID> vertexIDs, precice::span<const double> values);

    // Steering methods
    void initialize();

    void advance(double computedTimeStepSize);

    void finalize();

    // Status queries
    bool isCouplingOngoing();
    bool requiresInitialData() const;
    bool requiresWritingCheckpoint() const;
    bool requiresReadingCheckpoint() const;

    bool isTimeWindowComplete();
    double getMaxTimeStepSize();

    // Profiling
    void startProfilingSection(const std::string &name);

    void stopLastProfilingSection();

  private:
    void constructFluidParticipantMeshes();
    void constructSolidParticipantMeshes();

    int getSolidConnectionSocket() const;
    int getFluidConnectionSocket() const;

    void sendMeshVertices() const;
    void receiveMeshVertices() const;
    void computeMappings();
    void mapWriteData();
    void mapReadData();
    std::string _participantName;
    std::string _remoteParticipantName;
    std::string _configFileName;
    CouplingScheme _couplingScheme;
    std::string _participantMeshName;
    std::string _remoteParticipantMeshName;
    std::string _participantDataName;
    std::string _remoteParticipantDataName;
    int _rank;
    int _size;
    int _remoteSocket;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> _meshes;
};
} // namespace MinimalCoupler