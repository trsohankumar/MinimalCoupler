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

        int getMeshDimensions(
            precice::string_view meshName) const override;

        void setMeshVertices(
            precice::string_view meshName,
            precice::span<const double> coordinates,
            precice::span<int> ids) override;

        void readData(
            const std::string &meshName,
            const std::string &dataName,
            const std::vector<int> &vertexIDs,
            double relativeReadTime,
            std::vector<double> &values) const override;

        void writeData(
            const std::string &meshName,
            const std::string &dataName,
            const std::vector<int> &vertexIDs,
            const std::vector<double> &values) override;

        void initialize() override;

        void advance(
            double computedTimeStepSize) override;

        void finalize() override;

        // Status queries
        bool isCouplingOngoing() const override;
        bool requiresInitialData() const override;
        bool requiresWritingCheckpoint() const override;
        bool requiresReadingCheckpoint() const override;

        double getMaxTimeStepSize() const override;

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