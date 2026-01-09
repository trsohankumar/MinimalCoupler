#include "FluidParticipantImpl.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include "data/Point.hpp"
#include "logger/logger.hpp"

namespace MinimalCoupler
{

    FluidParticipantImplementation::FluidParticipantImplementation(
        precice::string_view participantName,
        precice::string_view configurationFileName,
        int solverProcessIndex,
        int solverProcessSize)
        : ParticipantImplementation(participantName, configurationFileName, solverProcessIndex, solverProcessSize), solidSocket(-1)
    {
        
        // setup coupling scheme max Time an window size
        getCouplingScheme().setMaxTime(1.0);
        getCouplingScheme().setTimeWindowSize(0.1);

        // Fluid has two meshes one that it provdes and another that it recieves from the Solid. Both have two data vectors
        auto providedMesh = std::make_unique<Mesh>();
        providedMesh->setMeshName(getParticipantName() + "-Mesh");
        providedMesh->addDataToMesh("Force", getCouplingScheme().getCurrentTime());
        providedMesh->addDataToMesh("Displacement", getCouplingScheme().getCurrentTime());
        providedMesh->setMeshDimensions(2);

        _meshes[std::string(providedMesh->getMeshName())] = std::move(providedMesh);

        auto receivedMesh = std::make_unique<Mesh>();
        receivedMesh->setMeshName(getRemoteParticipantName() + "-Mesh");
        receivedMesh->addDataToMesh("Force", getCouplingScheme().getCurrentTime());
        receivedMesh->addDataToMesh("Displacement", getCouplingScheme().getCurrentTime());
        receivedMesh->setMeshDimensions(2);

        _meshes[std::string(receivedMesh->getMeshName())] = std::move(receivedMesh);

        // setup logging into a file
        Logger::getInstance().setLogFile("Fluid.log");
    }

    void FluidParticipantImplementation::initialize()
    {
        // 1. Preprocess any meshes
        MINIMALCOUPLER_INFO("Starting initialization...");

        // 2. Setup communication between Fluid and solid participants
        MINIMALCOUPLER_INFO("Waiting for Solid to connect...");
        solidSocket = getSolidConnectionSocket();
        MINIMALCOUPLER_INFO("Solid connected!");

        // 3. Transfer vertices from participant sender to receiver
        receiveMeshVertices();
        
        // compute mappings between meshes
        // The participant that receives the mesh MUST compute mappings in both the read and write directions
        computeMappings();
        // 4. Map Write Data
        mapWriteData();
        // 5. Initialize the coupling scheme
        getCouplingScheme().initialize(getParticipantName(), _meshes.at("Solid-Mesh").get(), solidSocket);

        // 6. Map Read Data
        mapReadData();

        MINIMALCOUPLER_INFO("Initialization complete!");
    }


    int FluidParticipantImplementation::getSolidConnectionSocket() const
    {
        // Fluid acts as a server in the TCP connects. So it binds to a port and Listens for solid connection
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(5001);
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Convert to network byte order

        if (bind(sock, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0)
        {
            close(sock);
            throw std::runtime_error("Failed to bind socket");
        }

        if (listen(sock, 1) < 0)
        {
            close(sock);
            throw std::runtime_error("Unable to listen on socket");
        }

        int client = accept(sock, nullptr, nullptr);
        if (client < 0)
        {
            close(sock);
            throw std::runtime_error("Unable to accept connection request from the client");
        }

        close(sock);
        return client;
    }

    void FluidParticipantImplementation::receiveMeshVertices() const
    {
        // recieves mesh vertices from the Solid Participant
        size_t size;
        recv(solidSocket, &size, sizeof(size), 0);
        MINIMALCOUPLER_INFO("Receiving ", size, " vertices from Solid");

        if (size > 0)
        {
            std::vector<Point> vertices(size);
            recv(solidSocket, vertices.data(), size * sizeof(Point), MSG_WAITALL);

            MINIMALCOUPLER_INFO("Solid mesh vertices set.");
            _meshes.at("Solid-Mesh")->setMeshVertices(vertices);
            _meshes.at("Solid-Mesh")->allocateDataFields();
        }
    }

    int FluidParticipantImplementation::getMeshDimensions(
        precice::string_view meshName) const
    {
        return _meshes.at(std::string(meshName))->getMeshDimensions();
    }

    void FluidParticipantImplementation::setMeshVertices(
        precice::string_view meshName,
        precice::span<const double> coordinates,
        precice::span<int> ids)
    {
        // recieves flat point vector from user and contructs a vector of points for the participants mesh
        auto &mesh = _meshes.at(std::string(meshName));
        int dim = mesh->getMeshDimensions();

        std::vector<Point> vertices;
        vertices.reserve(coordinates.size() / dim);

        for (size_t i = 0; i < coordinates.size(); i += dim)
        {

            ids[i / dim] = static_cast<precice::VertexID>(i / dim);
            Point v {ids[i/dim], coordinates[i], coordinates[i + 1]};
            vertices.emplace_back(std::move(v));
        }

        mesh->setMeshVertices(vertices);
        mesh->allocateDataFields();
    }

    void FluidParticipantImplementation::computeMappings()
    {

        // Here we compute vertex mapping from Solid -> FLuid mesh and Fluid-> Solid. This will help in further read and write data mappings
        MINIMALCOUPLER_INFO("Computing mappings between Fluid and Solid meshes vertices");
        NearestNeighbor nnMapper;

        const auto& fluidMeshVertices = _meshes.at("Fluid-Mesh")->getMeshVertices();
        const auto& solidMeshVertices = _meshes.at("Solid-Mesh")->getMeshVertices();

        auto fluidToSolidMapping = nnMapper.computeNearestNeighbors(solidMeshVertices, fluidMeshVertices);

        MINIMALCOUPLER_FILE_INFO("Fluid to Solid Mapping (write):");
        for(const auto &m: fluidToSolidMapping)
        {
            MINIMALCOUPLER_FILE_INFO("Fluid Vertex: (", fluidMeshVertices[m.id].x, ",", fluidMeshVertices[m.id].y, ") maps to Solid Vertex ", m.id, ": (", m.x, ", ", m.y, ")");
        }

        MINIMALCOUPLER_INFO("Computing mappings between Solid and Fluid meshes vertices");
        auto solidToFluidMapping = nnMapper.computeNearestNeighbors(fluidMeshVertices, solidMeshVertices);

        MINIMALCOUPLER_FILE_INFO("Solid to Fluid Mapping (read):");
        for(const auto &m: solidToFluidMapping)
        {
            MINIMALCOUPLER_FILE_INFO("Solid Vertex: (", solidMeshVertices[m.id].x, ",", solidMeshVertices[m.id].y, ") maps to Fluid Vertex ", m.id, ": (", m.x, ", ", m.y, ")");
        }

        _meshes.at("Fluid-Mesh")->setWriteMapping(std::move(fluidToSolidMapping));
        _meshes.at("Fluid-Mesh")->setReadMapping(std::move(solidToFluidMapping));
    }

    void FluidParticipantImplementation::mapWriteData()
    {
        // Before sending force data to solid, the data is mapped from FLuid mesh to solid mesh
        const auto& fluidForceData = _meshes.at("Fluid-Mesh")->getDataField("Force", getCouplingScheme().getCurrentTime());

        auto& solidForceData = _meshes.at("Solid-Mesh")->getDataField("Force", getCouplingScheme().getCurrentTime());

        NearestNeighbor::mapConservative(_meshes.at("Fluid-Mesh")->getWriteMapping(), fluidForceData, solidForceData, _meshes.at("Fluid-Mesh")->getMeshDimensions());

        MINIMALCOUPLER_INFO("Mapping force data from Fluid to Solid");
        MINIMALCOUPLER_FILE_INFO("Mapping force data from Fluid to Solid");
        int dim = _meshes.at("Solid-Mesh")->getMeshDimensions();
        for (size_t i = 0; i < solidForceData.size(); i += dim) {
            MINIMALCOUPLER_FILE_INFO("Vertex ", i/dim, ": (",  solidForceData[i], ", ", solidForceData[i+1], ")");
        }
    }
    
    void FluidParticipantImplementation::mapReadData()
    {
        // After receiving displacement data from solid, disp data is mapped from Solid mesh to Fluid mesh
        auto& fluidDispData = _meshes.at("Fluid-Mesh")->getDataField("Displacement", getCouplingScheme().getCurrentTime());

        const auto& solidDispData = _meshes.at("Solid-Mesh")->getDataField("Displacement", getCouplingScheme().getCurrentTime());

        NearestNeighbor::mapConsistent(_meshes.at("Fluid-Mesh")->getReadMapping(), solidDispData, fluidDispData, _meshes.at("Fluid-Mesh")->getMeshDimensions());

        MINIMALCOUPLER_INFO("Mapping displacement data from Solid mesh to Fluid mesh");
        MINIMALCOUPLER_FILE_INFO("Mapping displacement data from Solid mesh to Fluid mesh");
        int dim = _meshes.at("Fluid-Mesh")->getMeshDimensions();
        for (size_t i = 0; i < fluidDispData.size(); i += dim) {
            
            MINIMALCOUPLER_FILE_INFO("Vertex ", i/dim, ": (", fluidDispData[i], ", ", fluidDispData[i+1], ")");
        }
    }

    void FluidParticipantImplementation::readData(
        const std::string &meshName,
        const std::string &dataName,
        const std::vector<int> &vertexIDs,
        double relativeReadTime,
        std::vector<double> &values) const
    {
    }

    void FluidParticipantImplementation::writeData(
        const std::string &meshName,
        const std::string &dataName,
        const std::vector<int> &vertexIDs,
        const std::vector<double> &values)
    {
    }

    void FluidParticipantImplementation::advance(
        double computedTimeStepSize)
    {
    }

    void FluidParticipantImplementation::finalize()
    {
        close(solidSocket);
    }

    bool FluidParticipantImplementation::isCouplingOngoing() const
    {
        return false;
    }

    bool FluidParticipantImplementation::requiresInitialData() const
    {
        return false;
    }

    bool FluidParticipantImplementation::requiresWritingCheckpoint() const
    {
        return false;
    }

    bool FluidParticipantImplementation::requiresReadingCheckpoint() const
    {
        return false;
    }

    double FluidParticipantImplementation::getMaxTimeStepSize() const
    {
        return 1.0;
    }

    void FluidParticipantImplementation::startProfilingSection(
        const std::string &name)
    {
    }

    void FluidParticipantImplementation::stopLastProfilingSection()
    {
    }
}