#include "FluidParticipantImpl.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include "data/Point.hpp"

namespace MinimalCoupler
{

    FluidParticipantImplementation::FluidParticipantImplementation(precice::string_view participantName, precice::string_view configurationFileName, int solverProcessIndex, int solverProcessSize)
        : ParticipantImplementation(std::string(participantName), std::string(configurationFileName), solverProcessIndex, solverProcessSize), solidSocket(-1)
    {
        getCouplingScheme().setMaxTime(1.0);
        getCouplingScheme().setTimeWindowSize(0.1);

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
    }

    void FluidParticipantImplementation::initialize()
    {
        // 1. Preprocess any meshes
        std::cout << "[FLUID] Starting initialization..." << std::endl;

        // 2. Setup communication between Fluid and solid participants
        std::cout << "[FLUID] Waiting for Solid to connect..." << std::endl;
        solidSocket = getSolidConnectionSocket();
        std::cout << "[FLUID] Solid connected! Socket: " << solidSocket << std::endl;

        // 3. Transfer vertices from participant sender to receiver
        sendMeshVertices();
        
        // compute mappings between meshes
        // The participant that receives the mesh MUST compute mappings in both the read and write directions
        computeMappings();
        // 4. Map Write Data
        mapWriteData();
        // 5. Initialize the coupling scheme


        // 6. Map Read Data
        // mapReadData();
        std::cout << "[FLUID] Initialization complete!" << std::endl;
    }


    int FluidParticipantImplementation::getSolidConnectionSocket() const
    {
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

    void FluidParticipantImplementation::sendMeshVertices() const
    {
        size_t size;
        recv(solidSocket, &size, sizeof(size), 0);
        std::cout << "[FLUID] Receiving " << size << " vertices from Solid" << std::endl;

        if (size > 0)
        {
            std::vector<Point> vertices(size);
            recv(solidSocket, vertices.data(), size * sizeof(Point), MSG_WAITALL);

            std::cout << "[FLUID] Solid mesh vertices set." << std::endl;
            _meshes.at("Solid-Mesh")->setMeshVertices(vertices);
            _meshes.at("Solid-Mesh")->allocateDataFields();

            std::cout << "[FLUID] Vertices received:" << std::endl;
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                std::cout << "  Vertex " << i << ": (" << vertices[i].x << ", " << vertices[i].y << ")" << std::endl;
            }
            std::cout << "[FLUID] Solid mesh vertices set." << std::endl;
        }
    }

    int FluidParticipantImplementation::getMeshDimensions(precice::string_view meshName) const
    {
        return _meshes.at(std::string(meshName))->getMeshDimensions();
    }

    void FluidParticipantImplementation::setMeshVertices(precice::string_view meshName, precice::span<const double> coordinates, precice::span<VertexID> ids)
    {
        auto &mesh = _meshes.at(std::string(meshName));
        int dim = mesh->getMeshDimensions();

        std::vector<Point> vertices;
        vertices.reserve(coordinates.size() / dim);

        for (size_t i = 0; i < coordinates.size(); i += dim)
        { 

            ids[i / dim] = static_cast<VertexID>(i / dim);
            Point v {ids[i/dim], coordinates[i], coordinates[i + 1]};
            vertices.emplace_back(std::move(v));
        }

        mesh->setMeshVertices(vertices);
        mesh->allocateDataFields();
    }

    void FluidParticipantImplementation::computeMappings()
    {
        std::cout << "[FLUID] Computing mappings between Fluid and Solid meshes..." << std::endl;
        NearestNeighbor nnMapper;

        const auto& fluidMeshVertices = _meshes.at("Fluid-Mesh")->getMeshVertices();
        const auto& solidMeshVertices = _meshes.at("Solid-Mesh")->getMeshVertices();

        auto fluidToSolidMapping = nnMapper.computeNearestNeighbors(solidMeshVertices, fluidMeshVertices);

        std::cout << "[FLUID] Fluid to Solid Mapping (write):" << std::endl;
        for(const auto &m: fluidToSolidMapping)
        {
            std::cout << "  Fluid Vertex maps to Solid Vertex " << m.id << ": (" << m.x << ", " << m.y << ")" << std::endl;
        }

        auto solidToFluidMapping = nnMapper.computeNearestNeighbors(fluidMeshVertices, solidMeshVertices);

        std::cout << "[FLUID] Solid to Fluid Mapping (read):" << std::endl;
        for(const auto &m: solidToFluidMapping)
        {
            std::cout << "  Solid Vertex maps to Fluid Vertex " << m.id << ": (" << m.x << ", " << m.y << ")" << std::endl;
        }

        _meshes.at("Fluid-Mesh")->setWriteMapping(std::move(fluidToSolidMapping));
        _meshes.at("Fluid-Mesh")->setReadMapping(std::move(solidToFluidMapping));
    }

    void FluidParticipantImplementation::mapWriteData()
    {
        const auto& fluidForceData = _meshes.at("Fluid-Mesh")->getDataField("Force", getCouplingScheme().getCurrentTime());

        auto& solidForceData = _meshes.at("Solid-Mesh")->getDataField("Force", getCouplingScheme().getCurrentTime());

        NearestNeighbor::mapConservative(_meshes.at("Fluid-Mesh")->getWriteMapping(), fluidForceData, solidForceData, _meshes.at("Fluid-Mesh")->getMeshDimensions());

        std::cout << "[FLUID] Solid mesh Force data after mapping:" << std::endl;
        int dim = _meshes.at("Solid-Mesh")->getMeshDimensions();
        for (size_t i = 0; i < solidForceData.size(); i += dim) {
            std::cout << "  Vertex " << i/dim << ": (" << solidForceData[i] << ", " << solidForceData[i+1] << ")" << std::endl;
        }
    }
    
    void FluidParticipantImplementation::mapReadData()
    {
        auto& fluidDispData = _meshes.at("Fluid-Mesh")->getDataField("Displacement", getCouplingScheme().getCurrentTime());

        const auto& solidDispData = _meshes.at("Solid-Mesh")->getDataField("Displacement", getCouplingScheme().getCurrentTime());

        NearestNeighbor::mapConservative(_meshes.at("Fluid-Mesh")->getReadMapping(), solidDispData, fluidDispData, _meshes.at("Fluid-Mesh")->getMeshDimensions());
    }

    void FluidParticipantImplementation::readData(const std::string &meshName, const std::string &dataName, const std::vector<int> &vertexIDs, double relativeReadTime, std::vector<double> &values) const
    {
    }

    void FluidParticipantImplementation::writeData(const std::string &meshName, const std::string &dataName, const std::vector<int> &vertexIDs, const std::vector<double> &values)
    {
    }

    void FluidParticipantImplementation::advance(double computedTimeStepSize)
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

    void FluidParticipantImplementation::startProfilingSection(const std::string &name)
    {
    }

    void FluidParticipantImplementation::stopLastProfilingSection()
    {
    }
}