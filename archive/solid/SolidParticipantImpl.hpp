#pragma once

#include <unordered_map>
#include "data/Mesh.hpp"
#include "impl/ParticipantImpl.hpp"

namespace MinimalCoupler
{
    class SolidParticipantImplementation : public ParticipantImplementation
    {
    public:
        SolidParticipantImplementation(
            precice::string_view participantName,
            precice::string_view configurationFileName,
            int solverProcessIndex,
            int solverProcessSize);

        SolidParticipantImplementation() = default;
        ~SolidParticipantImplementation() override;

        // Mesh methods
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

        // Steering methods
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
        int getFluidConnectionSocket() const;
        void sendMeshVertices() const;
        std::unordered_map<std::string, std::unique_ptr<Mesh>> _meshes;
        int fluidSocket;
    };
}