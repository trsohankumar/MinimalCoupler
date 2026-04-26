#include "precice/Participant.hpp"

#include "../ParticipantImpl.hpp"

#include "../logger.hpp"
#include <stdexcept>

namespace precice::impl {

class ParticipantImpl : public MinimalCoupler::ParticipantImplementation {
public:
    using MinimalCoupler::ParticipantImplementation::ParticipantImplementation;
};

} // namespace precice::impl

namespace precice {

Participant::Participant(
    precice::string_view participantName,
    precice::string_view configurationFileName,
    int                  solverProcessIndex,
    int                  solverProcessSize
)
{
    if (participantName == "Solid" || participantName == "Fluid") {
        _impl = std::make_unique<impl::ParticipantImpl>(
            participantName,
            configurationFileName,
            solverProcessIndex,
            solverProcessSize
        );
    } else {
        throw std::invalid_argument("Unknown participant name:" + std::string(participantName));
    }
}

Participant::~Participant() { }

int Participant::getMeshDimensions(precice::string_view meshName) const
{
    return _impl->getMeshDimensions(meshName);
}

void Participant::setMeshVertices(
    precice::string_view        meshName,
    precice::span<const double> coordinates,
    precice::span<VertexID>     ids
)
{
    _impl->setMeshVertices(meshName, coordinates, ids);
}

void Participant::readData(
    precice::string_view                   meshName,
    precice::string_view                   dataName,
    precice::span<const precice::VertexID> ids,
    double                                 relativeReadTime,
    precice::span<double>                  values
) const
{
    _impl->readData(meshName, dataName, ids, relativeReadTime, values);
}

void Participant::writeData(
    precice::string_view          meshName,
    precice::string_view          dataName,
    precice::span<const VertexID> ids,
    precice::span<const double>   values
)
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
    MINIMALCOUPLER_WARNING("Not implemented: startProfilingSection");
}

void Participant::stopLastProfilingSection()
{
    MINIMALCOUPLER_WARNING("Not implemented: stopLastProfilingSection");
}

// Constructor with MPI communicator
Participant::Participant(
    precice::string_view participantName,
    precice::string_view configurationFileName,
    int                  solverProcessIndex,
    int                  solverProcessSize,
    void*                communicator
)
{
    MINIMALCOUPLER_WARNING("Not implemented: Participant MPI constructor");
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
    MINIMALCOUPLER_WARNING("Not implemented: requiresMeshConnectivityFor");
    return false;
}

void Participant::resetMesh(precice::string_view meshName)
{
    MINIMALCOUPLER_WARNING("Not implemented: resetMesh");
}

VertexID Participant::setMeshVertex(precice::string_view meshName, precice::span<const double> position)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshVertex");
    return 0; // TODO: implement properly
}

int Participant::getMeshVertexSize(precice::string_view meshName) const
{
    MINIMALCOUPLER_WARNING("Not implemented: getMeshVertexSize");
    return 0;
}

void Participant::setMeshEdge(precice::string_view meshName, VertexID first, VertexID second)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshEdge");
}

void Participant::setMeshEdges(precice::string_view meshName, precice::span<const VertexID> ids)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshEdges");
}

void Participant::setMeshTriangle(precice::string_view meshName, VertexID first, VertexID second, VertexID third)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshTriangle");
}

void Participant::setMeshTriangles(precice::string_view meshName, precice::span<const VertexID> ids)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshTriangles");
}

void Participant::setMeshQuad(
    precice::string_view meshName,
    VertexID             first,
    VertexID             second,
    VertexID             third,
    VertexID             fourth
)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshQuad");
}

void Participant::setMeshQuads(precice::string_view meshName, precice::span<const VertexID> ids)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshQuads");
}

void Participant::setMeshTetrahedron(
    precice::string_view meshName,
    VertexID             first,
    VertexID             second,
    VertexID             third,
    VertexID             fourth
)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshTetrahedron");
}

void Participant::setMeshTetrahedra(precice::string_view meshName, precice::span<const VertexID> ids)
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshTetrahedra");
}

// Advanced data methods
bool Participant::requiresGradientDataFor(precice::string_view meshName, precice::string_view dataName) const
{
    MINIMALCOUPLER_WARNING("Not implemented: requiresGradientDataFor");
    return false;
}

void Participant::writeGradientData(
    precice::string_view          meshName,
    precice::string_view          dataName,
    precice::span<const VertexID> ids,
    precice::span<const double>   gradients
)
{
    MINIMALCOUPLER_WARNING("Not implemented: writeGradientData");
}

void Participant::mapAndReadData(
    precice::string_view        fromMeshName,
    precice::string_view        dataName,
    precice::span<const double> positions,
    double                      relativeReadTime,
    precice::span<double>       values
) const
{
    MINIMALCOUPLER_WARNING("Not implemented: mapAndReadData");
}

void Participant::writeAndMapData(
    precice::string_view        meshName,
    precice::string_view        dataName,
    precice::span<const double> positions,
    precice::span<const double> values
)
{
    MINIMALCOUPLER_WARNING("Not implemented: writeAndMapData");
}

// Direct access methods
void Participant::setMeshAccessRegion(precice::string_view meshName, precice::span<const double> boundingBox) const
{
    MINIMALCOUPLER_WARNING("Not implemented: setMeshAccessRegion");
}

void Participant::getMeshVertexIDsAndCoordinates(
    precice::string_view    meshName,
    precice::span<VertexID> ids,
    precice::span<double>   coordinates
) const
{
    MINIMALCOUPLER_WARNING("Not implemented: getMeshVertexIDsAndCoordinates");
}

} // namespace precice