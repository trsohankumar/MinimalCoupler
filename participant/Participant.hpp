#pragma once

#include <string>
#include <vector>
#include <memory>
#include "impl/ParticipantImpl.hpp"
#include "precice/types.hpp"

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
    // Construction and Configuration
    Participant(
        precice::string_view participantName,
        precice::string_view configurationFileName,
        int solverProcessIndex,
        int solverProcessSize);

    Participant(
        precice::string_view participantName,
        precice::string_view configurationFileName,
        int solverProcessIndex,
        int solverProcessSize,
        void *communicator);

    ~Participant();

    // Status Queries
    int getMeshDimensions(
        precice::string_view meshName) const;

    int getDataDimensions(
        precice::string_view meshName,
        precice::string_view dataName) const;

    bool isCouplingOngoing() const;

    bool isTimeWindowComplete() const;

    double getMaxTimeStepSize() const;

    // Mesh Access
    bool requiresMeshConnectivityFor(
        precice::string_view meshName) const;

    void resetMesh(
        precice::string_view meshName);

    VertexID setMeshVertex(
        precice::string_view meshName,
        precice::span<const double> position);

    int getMeshVertexSize(
        precice::string_view meshName) const;

    void setMeshVertices(
        precice::string_view meshName,
        precice::span<const double> coordinates,
        precice::span<VertexID> ids);

    void setMeshEdge(
        precice::string_view meshName,
        VertexID first,
        VertexID second);

    void setMeshEdges(
        precice::string_view meshName,
        precice::span<const VertexID> ids);

    void setMeshTriangle(
        precice::string_view meshName,
        VertexID first,
        VertexID second,
        VertexID third);

    void setMeshTriangles(
        precice::string_view meshName,
        precice::span<const VertexID> ids);

    void setMeshQuad(
        precice::string_view meshName,
        VertexID first,
        VertexID second,
        VertexID third,
        VertexID fourth);

    void setMeshQuads(
        precice::string_view meshName,
        precice::span<const VertexID> ids);

    void setMeshTetrahedron(
        precice::string_view meshName,
        VertexID first,
        VertexID second,
        VertexID third,
        VertexID fourth);

    void setMeshTetrahedra(
        precice::string_view meshName,
        precice::span<const VertexID> ids);

    // Data access methods
    bool requiresInitialData();

    bool requiresGradientDataFor(
        precice::string_view meshName,
        precice::string_view dataName) const;

    void readData(
        precice::string_view meshName,
        precice::string_view dataName,
        precice::span<const VertexID> ids,
        double relativeReadTime,
        precice::span<double> values) const;

    void writeData(
        precice::string_view meshName,
        precice::string_view dataName,
        precice::span<const VertexID> ids,
        precice::span<const double> values);

    void writeGradientData(
        precice::string_view meshName,
        precice::string_view dataName,
        precice::span<const VertexID> ids,
        precice::span<const double> gradients);

    void mapAndReadData(
        precice::string_view fromMeshName,
        precice::string_view dataName,
        precice::span<const double> positions,
        double relativeReadTime,
        precice::span<double> values) const;

    void writeAndMapData(
        precice::string_view meshName,
        precice::string_view dataName,
        precice::span<const double> positions,
        precice::span<const double> values);

    // Direct access
    void setMeshAccessRegion(
        precice::string_view meshName,
        precice::span<const double> boundingBox) const;

    void getMeshVertexIDsAndCoordinates(
        precice::string_view meshName,
        precice::span<VertexID> ids,
        precice::span<double> coordinates) const;

    // Steering methods
    void initialize();

    void advance(
        double computedTimeStepSize);

    void finalize();

    bool requiresWritingCheckpoint();

    bool requiresReadingCheckpoint();

    // Profiling
    void startProfilingSection(
        precice::string_view name);

    void stopLastProfilingSection();

  private:
    std::unique_ptr<MinimalCoupler::ParticipantImplementation> _impl;
  };
}