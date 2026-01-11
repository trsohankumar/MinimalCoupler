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

        // Mesh methods
        int getMeshDimensions(
            precice::string_view meshName) const override;

        void setMeshVertices(
            precice::string_view meshName,
            precice::span<const double> coordinates,
            precice::span<int> ids) override;

        // Data exchange methods
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