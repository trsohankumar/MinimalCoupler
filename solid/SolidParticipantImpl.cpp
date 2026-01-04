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
        getCouplingScheme().setMaxTime(1.0);
        getCouplingScheme().setTimeWindowSize(0.1);

        std::cout << "Starting constructor of Solid" << std::endl;
        auto providedMesh = std::make_unique<Mesh>();
        providedMesh->setMeshName(getParticipantName() + "-Mesh");
        providedMesh->addDataToMesh("Force", getCouplingScheme().getCurrentTime());
        providedMesh->addDataToMesh("Displacement", getCouplingScheme().getCurrentTime());
        providedMesh->setMeshDimensions(2);

        std::string meshKey = std::string(providedMesh->getMeshName());
        std::cout << "Inserting mesh with key: '" << meshKey << "'" << std::endl;
        _meshes[meshKey] = std::move(providedMesh);

        std::cout << "Finished constructor of Solid" << std::endl;
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
        sendMeshVertices();

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
        serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  

        // Try to implement a retry mechanism here
        bool connected = false;
        while (!connected)
        {
            if (connect(sock, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == 0)
            {
                connected = true;
            }
            else
            {
                std::cout << "[SOLID] Connection to Fluid failed, retrying in 1 second..." << std::endl;
                sleep(1); 
            }
        }
                
        return sock;
    }

    void SolidParticipantImplementation::sendMeshVertices() const
    {
        std::string meshName = getParticipantName() + "-Mesh";
        std::cout << "Looking up mesh with key: '" << meshName << "'" << std::endl;
        size_t size = _meshes.at(meshName)->getVertexCount();
        std::cout << "[SOLID] Sending " << size << " vertices" << std::endl;
        send(fluidSocket, &size, sizeof(size), 0);

        if (size > 0)
        {
            auto vertices = _meshes.at(meshName)->getMeshVertices();

            std::cout << "[SOLID] Vertices being sent:" << std::endl;
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                std::cout << "  Vertex " << i << ": (" << vertices[i].x << ", " << vertices[i].y << ")" << std::endl;
            }

            send(fluidSocket, vertices.data(), size * sizeof(Point), 0);
        }

    }

    
    int SolidParticipantImplementation::getMeshDimensions(precice::string_view meshName) const
    {
        std::cout<<"Here in getMeshDimensions for Solid" << std::endl;
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
            ids[i / dim] = static_cast<VertexID>(i / dim);
            Point v {ids[i/dim], coordinates[i], coordinates[i + 1]};
            vertices.emplace_back(std::move(v));
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