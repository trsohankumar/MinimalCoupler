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
        auto providedMesh = std::make_unique<Mesh>();
        providedMesh->setMeshName(_participantName + "-Mesh");
        providedMesh->addDataToMesh("Force");
        providedMesh->addDataToMesh("Displacement");
        providedMesh->setMeshDimensions(2);

        _meshes[std::string(providedMesh->getMeshName())] = std::move(providedMesh);

        auto receivedMesh = std::make_unique<Mesh>();
        receivedMesh->setMeshName(_remoteParticipantName + "-Mesh");
        receivedMesh->addDataToMesh("Force");
        receivedMesh->addDataToMesh("Displacement");
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
        // 4. Initialize the coupling scheme

        // 5. Transfer Write Data

        // 6. Transfer Read Data
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
        // Implement mapping computation between fluid and solid meshes using NearestNeighbor
        std::cout << "[FLUID] Computing mappings between Fluid and Solid meshes..." << std::endl;
        NearestNeighbor nnSolidToFluidMapper, nnFluidToSolidMapper;
        
        const auto& fluidMeshVertices = _meshes.at("Fluid-Mesh")->getMeshVertices();
        const auto& solidMeshVertices = _meshes.at("Solid-Mesh")->getMeshVertices();

        auto mappedVertices = nnSolidToFluidMapper.computeNearestNeighbors(solidMeshVertices, fluidMeshVertices);

        std::cout << "[FLUID] Solid to Fluid Mapping i.e read:" << std::endl;
        for(const auto &r: mappedVertices)
        {
            std::cout << "  Solid Vertex " << r.id << " mapped to Fluid Vertex: (" << r.x << ", " << r.y << ")" << std::endl;
        }

        _meshes.at("Fluid-Mesh")->setVertexMapping(std::move(mappedVertices));
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