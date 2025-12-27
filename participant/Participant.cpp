#include "Participant.hpp"
#include "fluid/FluidParticipantImpl.hpp"
#include "solid/SolidParticipantImpl.hpp"

#include <stdexcept>
namespace precice
{

    Participant::Participant(::precice::string_view participantName, ::precice::string_view configurationFileName, int solverProcessIndex, int solverProcessSize)
    {
        if (participantName == "Fluid")
        {
            _impl = std::make_unique<MinimalCoupler::FluidParticipantImplementation>(
                std::move(participantName),
                std::move(configurationFileName),
                solverProcessIndex,
                solverProcessSize);
        }
        else if (participantName == "Solid")
        {

            _impl = std::make_unique<MinimalCoupler::SolidParticipantImplementation>(
                std::move(participantName),
                std::move(configurationFileName),
                solverProcessIndex,
                solverProcessSize);
        }
        else
        {
            throw std::invalid_argument("Unknown participant name:" + std::string(participantName));
        }
    }

    int Participant::getMeshDimensions(::precice::string_view meshName) const
    {
        return _impl->getMeshDimensions(meshName);
    }

    void Participant::setMeshVertices(::precice::string_view meshName,
                                      ::precice::span<const double> coordinates, ::precice::span<VertexID> ids)
    {
        _impl->setMeshVertices(meshName, coordinates, ids);
    }

    void Participant::readData(const std::string &meshName, const std::string &dataName, const std::vector<int> &vertexIDs, double relativeReadTime, std::vector<double> &values) const
    {
        _impl->readData(meshName, dataName, vertexIDs, relativeReadTime, values);
    }

    void Participant::writeData(const std::string &meshName, const std::string &dataName, const std::vector<int> &vertexIDs, const std::vector<double> &values)
    {
        _impl->writeData(meshName, dataName, vertexIDs, values);
    }

    void Participant::initialize()
    {
        _impl->initialize();
    }

    void Participant::advance(double computedTimeStepSize)
    {
        _impl->advance(computedTimeStepSize);
    }

    void Participant::finalize()
    {
        _impl->finalize();
    }

    bool Participant::isCouplingOngoing() const
    {
        return _impl->isCouplingOngoing();
    }

    bool Participant::requiresInitialData() const
    {
        return _impl->requiresInitialData();
    }

    bool Participant::requiresWritingCheckpoint() const
    {
        return _impl->requiresWritingCheckpoint();
    }

    bool Participant::requiresReadingCheckpoint() const
    {
        return _impl->requiresReadingCheckpoint();
    }

    double Participant::getMaxTimeStepSize() const
    {
        return _impl->getMaxTimeStepSize();
    }

    void Participant::startProfilingSection(const std::string &name)
    {
        _impl->startProfilingSection(name);
    }

    void Participant::stopLastProfilingSection()
    {
        _impl->stopLastProfilingSection();
    }
} // namespace precice