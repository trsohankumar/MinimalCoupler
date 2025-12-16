#include "Participant.hpp"

namespace precice
{

Participant::Participant(
    std::string participantName,
    std::string configurationFileName,
    int solverProcessIndex,
    int solverProcessSize)
{
    // create appropriate participant based on participantName
    if (participantName == "Fluid")
    {
        _impl = std::make_unique<
    }
}

int Participant::getMeshDimensions(const std::string& meshName) const
{
    return 0;
}

void Participant::setMeshVertices(
    const std::string& meshName,
    const std::vector<double>& positions,
    std::vector<int>& ids)
{
}

void Participant::readData(
    const std::string& meshName,
    const std::string& dataName,
    const std::vector<int>& vertexIDs,
    double relativeReadTime,
    std::vector<double>& values) const
{
}

void Participant::writeData(
    const std::string& meshName,
    const std::string& dataName,
    const std::vector<int>& vertexIDs,
    const std::vector<double>& values)
{
}

void Participant::initialize()
{
}

void Participant::advance(double computedTimeStepSize)
{
}

void Participant::finalize()
{
}

bool Participant::isCouplingOngoing() const
{
    return false;
}

bool Participant::requiresInitialData() const
{
    return false;
}

bool Participant::requiresWritingCheckpoint() const
{
    return false;
}

bool Participant::requiresReadingCheckpoint() const
{
    return false;
}

double Participant::getMaxTimeStepSize() const
{
    return 0.0;
}

void Participant::startProfilingSection(const std::string& name)
{
}

void Participant::stopLastProfilingSection()
{
}

} // namespace precice