#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Aitken.hpp"
#include "Mesh.hpp"
#include "precice/Types.hpp"

namespace MinimalCoupler {
/**
 * @brief Implements a pariticpant that can couple the perpendicular flaps
 * tutorial.
 * 
 * The class is an implementation from scratch with the goal to identify 
 * the minimal functionality that must be provided by a librariy to 
 * support serial implicit coupling while implementing the preCICE API
 * 
 * The class only implements a subset of the participant functions
 * that are necessary for the perpendicular flaps tutorials.
 */
class ParticipantImplementation {
public:
    ParticipantImplementation(
        precice::string_view participantName,
        precice::string_view configurationFileName,
        int                  solverProcessIndex,
        int                  solverProcessSize
    );

    ~ParticipantImplementation();

    // Mesh methods
    int getMeshDimensions(precice::string_view meshName) const;
    int getDataDimensions(precice::string_view meshName, precice::string_view dataName) const;

    void
    setMeshVertices(precice::string_view meshName, precice::span<const double> coordinates, precice::span<int> ids);

    // Data exchange methods
    void readData(
        precice::string_view                   meshName,
        precice::string_view                   dataName,
        precice::span<const precice::VertexID> vertexIDs,
        double                                 relativeReadTime,
        precice::span<double>                  values
    ) const;

    void writeData(
        precice::string_view                   meshName,
        precice::string_view                   dataName,
        precice::span<const precice::VertexID> vertexIDs,
        precice::span<const double>            values
    );

    // Steering methods
    void initialize();

    void advance(double computedTimeStepSize);

    void finalize();

    // Status queries
    bool isCouplingOngoing();
    bool requiresInitialData();
    bool requiresWritingCheckpoint();
    bool requiresReadingCheckpoint();

    bool   isTimeWindowComplete();
    double getMaxTimeStepSize();

private:
    void constructFluidParticipantMeshes();
    void constructSolidParticipantMeshes();

    int getSolidConnectionSocket() const;
    int getFluidConnectionSocket() const;

    void sendMeshVertices() const;
    void receiveMeshVertices();
    void computeMappings();
    void mapWriteData();
    void mapReadData(int sourceWindow);

    void computeNearestNeighbors(const std::vector<Point>& queryPoints, const std::vector<Point>& searchSpaceOfPoints);

    double getCurrentTime() const;
    void   couplingSchemeInitialize();

    void couplingSchemeAdvance();

    void openConvergenceLog();
    void writeConvergenceEntry();
    void writeWatchpointEntry(double time, int window);
    void setupWatchPoint();

    bool               _requiresWritingCheckPoint;
    bool               _requiresReadingCheckPoint;
    bool               _timeWindowComplete;
    double             _maxTime;
    double             _timeWindowSize;
    int                _rank;
    int                _size;
    int                _remoteSocket;
    int                _watchpointVertexIndex;
    int                _currentTimeWindowNumber;
    std::string        _participantName;
    std::string        _remoteParticipantName;
    std::string        _configFileName;
    std::string        _participantMeshName;
    std::string        _remoteParticipantMeshName;
    std::string        _participantDataName;
    std::string        _remoteParticipantDataName;
    Mesh               _providedMesh;
    Mesh               _receivedMesh;
    Aitken             _aitken;
    std::vector<Point> _vertexMapping;
    std::ofstream      _convergenceLog;
    std::ofstream      _watchpointLog;
};
} // namespace MinimalCoupler