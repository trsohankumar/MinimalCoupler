#include "precice/Participant.hpp"

#include "../ParticipantImpl.hpp"

#include <iostream>
#include <stdexcept>

namespace precice
{

Participant::Participant(precice::string_view participantName, precice::string_view configurationFileName, int solverProcessIndex, int solverProcessSize)
{
    if (participantName == "Solid" || participantName == "Fluid")
    {
        _impl = std::make_unique<MinimalCoupler::ParticipantImplementation>(std::move(participantName), std::move(configurationFileName), solverProcessIndex, solverProcessSize);
    }
    else
    {
        throw std::invalid_argument("Unknown participant name:" + std::string(participantName));
    }
}

Participant::~Participant()
{
}

int Participant::getMeshDimensions(precice::string_view meshName) const
{
    return _impl->getMeshDimensions(meshName);
}

void Participant::setMeshVertices(precice::string_view meshName, precice::span<const double> coordinates,
                                  precice::span<VertexID> ids)
{
    _impl->setMeshVertices(meshName, coordinates, ids);
}

void Participant::readData(precice::string_view meshName, precice::string_view dataName,precice::span<const precice::VertexID> ids, double relativeReadTime, precice::span<double> values) const
{
    _impl->readData(meshName, dataName, ids, relativeReadTime, values);
}

void Participant::writeData(precice::string_view meshName, precice::string_view dataName,precice::span<const VertexID> ids, precice::span<const double> values)
{
    _impl->writeData(meshName, dataName, ids, values);
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

bool Participant::requiresInitialData()
{
    return _impl->requiresInitialData();
}

bool Participant::requiresWritingCheckpoint()
{
    return _impl->requiresWritingCheckpoint();
}

bool Participant::requiresReadingCheckpoint()
{
    return _impl->requiresReadingCheckpoint();
}

double Participant::getMaxTimeStepSize() const
{
    return _impl->getMaxTimeStepSize();
}

void Participant::startProfilingSection(precice::string_view name)
{
    std::cout << "Not implemented: startProfilingSection" << std::endl;
}

void Participant::stopLastProfilingSection()
{
    std::cout << "Not implemented: stopLastProfilingSection" << std::endl;
}

// Constructor with MPI communicator
Participant::Participant(precice::string_view participantName, precice::string_view configurationFileName, int solverProcessIndex, int solverProcessSize, void *communicator)
{
    std::cout << "Not implemented: Participant MPI constructor" << std::endl;
    throw std::runtime_error("MPI communicator constructor not implemented");
}

// Status query methods
int Participant::getDataDimensions(precice::string_view meshName, precice::string_view dataName) const
{
    return _impl->getDataDimensions(meshName, dataName);
}

bool Participant::isTimeWindowComplete() const
{
    return _impl->isTimeWindowComplete();
}

// Mesh topology methods
bool Participant::requiresMeshConnectivityFor(precice::string_view meshName) const
{
    std::cout << "Not implemented: requiresMeshConnectivityFor" << std::endl;
    return false;
}

void Participant::resetMesh(precice::string_view meshName)
{
    std::cout << "Not implemented: resetMesh" << std::endl;
}

VertexID Participant::setMeshVertex(precice::string_view meshName, precice::span<const double> position)
{
    std::cout<<"Not Implemented" <<std::endl;
    return 0; // TODO: implement properly
}

int Participant::getMeshVertexSize(precice::string_view meshName) const
{
    std::cout << "Not implemented: getMeshVertexSize" << std::endl;
    return 0;
}

void Participant::setMeshEdge(precice::string_view meshName, VertexID first, VertexID second)
{
    std::cout << "Not implemented: setMeshEdge" << std::endl;
}

void Participant::setMeshEdges(precice::string_view meshName, precice::span<const VertexID> ids)
{
    std::cout << "Not implemented: setMeshEdges" << std::endl;
}

void Participant::setMeshTriangle(precice::string_view meshName, VertexID first, VertexID second, VertexID third)
{
    std::cout << "Not implemented: setMeshTriangle" << std::endl;
}

void Participant::setMeshTriangles(precice::string_view meshName, precice::span<const VertexID> ids)
{
    std::cout << "Not implemented: setMeshTriangles" << std::endl;
}

void Participant::setMeshQuad(precice::string_view meshName, VertexID first, VertexID second, VertexID third,
                              VertexID fourth)
{
    std::cout << "Not implemented: setMeshQuad" << std::endl;
}

void Participant::setMeshQuads(precice::string_view meshName, precice::span<const VertexID> ids)
{
    std::cout << "Not implemented: setMeshQuads" << std::endl;
}

void Participant::setMeshTetrahedron(precice::string_view meshName, VertexID first, VertexID second, VertexID third,
                                     VertexID fourth)
{
    std::cout << "Not implemented: setMeshTetrahedron" << std::endl;
}

void Participant::setMeshTetrahedra(precice::string_view meshName, precice::span<const VertexID> ids)
{
    std::cout << "Not implemented: setMeshTetrahedra" << std::endl;
}

// Advanced data methods
bool Participant::requiresGradientDataFor(precice::string_view meshName, precice::string_view dataName) const
{
    std::cout << "Not implemented: requiresGradientDataFor" << std::endl;
    return false;
}

void Participant::writeGradientData(precice::string_view meshName, precice::string_view dataName,
                                    precice::span<const VertexID> ids, precice::span<const double> gradients)
{
    std::cout << "Not implemented: writeGradientData" << std::endl;
}

void Participant::mapAndReadData(precice::string_view fromMeshName, precice::string_view dataName,
                                 precice::span<const double> positions, double relativeReadTime,
                                 precice::span<double> values) const
{
    std::cout << "Not implemented: mapAndReadData" << std::endl;
}

void Participant::writeAndMapData(precice::string_view meshName, precice::string_view dataName,
                                  precice::span<const double> positions, precice::span<const double> values)
{
    std::cout << "Not implemented: writeAndMapData" << std::endl;
}

// Direct access methods
void Participant::setMeshAccessRegion(precice::string_view meshName, precice::span<const double> boundingBox) const
{
    std::cout << "Not implemented: setMeshAccessRegion" << std::endl;
}

void Participant::getMeshVertexIDsAndCoordinates(precice::string_view meshName, precice::span<VertexID> ids,
                                                 precice::span<double> coordinates) const
{
    std::cout << "Not implemented: getMeshVertexIDsAndCoordinates" << std::endl;
}

} // namespace precice