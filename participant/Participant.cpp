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

    Participant::~Participant()
    {

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

    void Participant::readData(::precice::string_view meshName, ::precice::string_view dataName, ::precice::span<const VertexID> ids, double relativeReadTime, ::precice::span<double> values) const
    {
        // TODO: implement with span-based signature
    }

    void Participant::writeData(::precice::string_view meshName, ::precice::string_view dataName, ::precice::span<const VertexID> ids, ::precice::span<const double> values)
    {
        // TODO: implement with span-based signature
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

    void Participant::startProfilingSection(::precice::string_view name)
    {
        // TODO: implement
    }

    void Participant::stopLastProfilingSection()
    {
        // TODO: implement
    }

    // Constructor with MPI communicator
    Participant::Participant(::precice::string_view participantName, ::precice::string_view configurationFileName, int solverProcessIndex, int solverProcessSize, void *communicator)
    {
        // For now, ignore the communicator and delegate to the basic constructor
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

    // Status query methods
    int Participant::getDataDimensions(::precice::string_view meshName, ::precice::string_view dataName) const
    {
        return 2; // TODO: implement properly
    }

    bool Participant::isTimeWindowComplete() const
    {
        return true; // TODO: implement properly
    }

    // Mesh topology methods
    bool Participant::requiresMeshConnectivityFor(::precice::string_view meshName) const
    {
        return false; // TODO: implement properly
    }

    void Participant::resetMesh(::precice::string_view meshName)
    {
        // TODO: implement
    }

    VertexID Participant::setMeshVertex(::precice::string_view meshName, ::precice::span<const double> position)
    {
        return 0; // TODO: implement properly
    }

    int Participant::getMeshVertexSize(::precice::string_view meshName) const
    {
        return 0; // TODO: implement properly
    }

    void Participant::setMeshEdge(::precice::string_view meshName, VertexID first, VertexID second)
    {
        // TODO: implement
    }

    void Participant::setMeshEdges(::precice::string_view meshName, ::precice::span<const VertexID> ids)
    {
        // TODO: implement
    }

    void Participant::setMeshTriangle(::precice::string_view meshName, VertexID first, VertexID second, VertexID third)
    {
        // TODO: implement
    }

    void Participant::setMeshTriangles(::precice::string_view meshName, ::precice::span<const VertexID> ids)
    {
        // TODO: implement
    }

    void Participant::setMeshQuad(::precice::string_view meshName, VertexID first, VertexID second, VertexID third, VertexID fourth)
    {
        // TODO: implement
    }

    void Participant::setMeshQuads(::precice::string_view meshName, ::precice::span<const VertexID> ids)
    {
        // TODO: implement
    }

    void Participant::setMeshTetrahedron(::precice::string_view meshName, VertexID first, VertexID second, VertexID third, VertexID fourth)
    {
        // TODO: implement
    }

    void Participant::setMeshTetrahedra(::precice::string_view meshName, ::precice::span<const VertexID> ids)
    {
        // TODO: implement
    }

    // Advanced data methods
    bool Participant::requiresGradientDataFor(::precice::string_view meshName, ::precice::string_view dataName) const
    {
        return false; // TODO: implement properly
    }

    void Participant::writeGradientData(::precice::string_view meshName, ::precice::string_view dataName, ::precice::span<const VertexID> ids, ::precice::span<const double> gradients)
    {
        // TODO: implement
    }

    void Participant::mapAndReadData(::precice::string_view fromMeshName, ::precice::string_view dataName, ::precice::span<const double> positions, double relativeReadTime, ::precice::span<double> values) const
    {
        // TODO: implement
    }

    void Participant::writeAndMapData(::precice::string_view meshName, ::precice::string_view dataName, ::precice::span<const double> positions, ::precice::span<const double> values)
    {
        // TODO: implement
    }

    // Direct access methods
    void Participant::setMeshAccessRegion(::precice::string_view meshName, ::precice::span<const double> boundingBox) const
    {
        // TODO: implement
    }

    void Participant::getMeshVertexIDsAndCoordinates(::precice::string_view meshName, ::precice::span<VertexID> ids, ::precice::span<double> coordinates) const
    {
        // TODO: implement
    }

} // namespace precice