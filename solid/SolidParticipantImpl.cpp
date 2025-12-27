#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <memory>

#include "SolidParticipantImpl.hpp"

namespace MinimalCoupler
{

    SolidParticipantImplementation::SolidParticipantImplementation(precice::string_view participantName,precice::string_view configurationFileName,int solverProcessIndex, int solverProcessSize)
        : ParticipantImplementation(std::string(participantName), std::string(configurationFileName), solverProcessIndex, solverProcessSize), fluidSocket(-1)
    {
        auto providedMesh = std::make_unique<Mesh>();
        providedMesh->setMeshName(_participantName + "-Mesh");
        providedMesh->addDataToMesh("Force");
        providedMesh->addDataToMesh("Displacement");
        providedMesh->setMeshDimensions(2);

        _meshes[std::string(providedMesh->getMeshName())] = std::move(providedMesh);


    }


    void SolidParticipantImplementation::initialize()
    {

        //1. Preprocess any meshes
        std::cout << "[SOLID] Starting initialization..." << std::endl;

        //2. Setup communication between Fluid and solid participants
        std::cout << "[SOLID] Connecting to Fluid..." << std::endl;
        fluidSocket = getFluidConnectionSocket();
        std::cout << "[SOLID] Connected! Socket: " << fluidSocket << std::endl;

        //3. Transfer vertices from participant sender to receiver
        char buffer[1024];

        int len = recv(fluidSocket, buffer, sizeof(buffer), 0);

        buffer[len] = '\0';
        std::cout << "[SOLID] Fluid says: " << buffer << std::endl;

        //4. Initialize the coupling scheme

        //5. Transfer Write Data

        //6. Transfer Read Data
        std::cout << "[SOLID] Initialization complete!" << std::endl;
    }

    int SolidParticipantImplementation::getFluidConnectionSocket() const
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family  = AF_INET;
        serverAddress.sin_port = htons(5001);
        serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // Convert to network byte order

        if (connect(sock, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0)
        {
            throw std::runtime_error("Unable to connect to Fluid participant");
        }
        
        return sock;
    }


    
    int SolidParticipantImplementation::getMeshDimensions(precice::string_view meshName) const
    {
        return _meshes.at(std::string(meshName))->getMeshDimensions();
    }

    void SolidParticipantImplementation::setMeshVertices(precice::string_view meshName, precice::span<const double> coordinates, precice::span<VertexID> ids)
    {
        auto& mesh = _meshes[std::string(meshName)];
        int dim = mesh->getMeshDimensions();

        std::vector<Point> vertices;
        vertices.reserve(coordinates.size() / dim);

        for (size_t i = 0; i < coordinates.size(); i += dim)
        {
            vertices.emplace_back(coordinates[i], coordinates[i+1], coordinates[i+2]);
            ids[i/dim] = static_cast<VertexID>(i/dim);
        }

        mesh->setMeshVertices(vertices);
        mesh->allocateDataFields();  // Allocates dimensions * vertexCount for each data field
    }

    // Data exchange methods
    void SolidParticipantImplementation::readData(const std::string& meshName, const std::string& dataName, const std::vector<int>& vertexIDs, double relativeReadTime, std::vector<double>& values) const
    {

    }

    void SolidParticipantImplementation::writeData( const std::string& meshName, const std::string& dataName, const std::vector<int>& vertexIDs, const std::vector<double>& values)
    {

    }

    void SolidParticipantImplementation::advance(double computedTimeStepSize)
    {

    }

    void SolidParticipantImplementation::finalize()
    {
        close(fluidSocket);
    }

    bool SolidParticipantImplementation::isCouplingOngoing() const
    {
        return false;
    }

    bool SolidParticipantImplementation::requiresInitialData() const
    {
        return false;
    }

    bool SolidParticipantImplementation::requiresWritingCheckpoint() const
    {
        return false;
    }

    bool SolidParticipantImplementation::requiresReadingCheckpoint() const
    {
        return false;
    }

    double SolidParticipantImplementation::getMaxTimeStepSize() const
    {
        return 1.0;
    }

    void SolidParticipantImplementation::startProfilingSection(const std::string& name)
    {

    }

    void SolidParticipantImplementation::stopLastProfilingSection()
    {

    }

}