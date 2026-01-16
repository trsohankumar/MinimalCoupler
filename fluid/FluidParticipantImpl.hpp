#pragma once

#include <memory>

#include "impl/ParticipantImpl.hpp"
#include "mapping/NearestNeighbor.hpp"
#include "data/Mesh.hpp"

namespace MinimalCoupler
{
    class FluidParticipantImplementation : public ParticipantImplementation
    {
    public:
        FluidParticipantImplementation(
            precice::string_view participantName,
            precice::string_view configurationFileName,
            int solverProcessIndex,
            int solverProcessSize);

        FluidParticipantImplementation() = default;
        ~FluidParticipantImplementation() override;

        int getMeshDimensions(
            precice::string_view meshName) const override;

        void setMeshVertices(
            precice::string_view meshName,
            precice::span<const double> coordinates,
            precice::span<int> ids) override;

        void readData(
            precice::string_view meshName,
            precice::string_view dataName,
            precice::span<const precice::VertexID> vertexIDs,
            double relativeReadTime,
            precice::span<double> values) const override;

        void writeData(
            precice::string_view meshName,
            precice::string_view dataName,
            precice::span<const precice::VertexID> vertexIDs,
            precice::span<const double> values) override;

        void initialize() override;

        void advance(
            double computedTimeStepSize) override;

        void finalize() override;

        // Status queries
        bool isCouplingOngoing() override;
        bool requiresInitialData() const override;
        bool requiresWritingCheckpoint() const override;
        bool requiresReadingCheckpoint() const override;

        bool isTimeWindowComplete() override;
        double getMaxTimeStepSize() override;

        // Profiling
        void startProfilingSection(
            const std::string &name) override;

        void stopLastProfilingSection() override;

    private:
        void receiveMeshVertices() const;

        int getSolidConnectionSocket() const;

        void computeMappings();

        void mapWriteData();

        void mapReadData();

        int solidSocket;
        std::unordered_map<std::string, std::unique_ptr<Mesh>> _meshes;
    };
}