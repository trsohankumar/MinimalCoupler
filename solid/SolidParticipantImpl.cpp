#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <memory>

#include "SolidParticipantImpl.hpp"
#include "logger/logger.hpp"

namespace MinimalCoupler
{

    SolidParticipantImplementation::SolidParticipantImplementation(
        precice::string_view participantName,
        precice::string_view configurationFileName,
        int solverProcessIndex,
        int solverProcessSize)
        : ParticipantImplementation(participantName, configurationFileName, solverProcessIndex, solverProcessSize), fluidSocket(-1)
    {
        // setup coupling scheme max Time an window size
        getCouplingScheme().setMaxTime(1.0);
        getCouplingScheme().setTimeWindowSize(0.1);

        // Solid only provides one mesh with two data vectors
        auto providedMesh = std::make_unique<Mesh>();
        providedMesh->setMeshName(getParticipantName() + "-Mesh");
        providedMesh->addDataToMesh("Force", getCouplingScheme().getCurrentTime());
        providedMesh->addDataToMesh("Displacement", getCouplingScheme().getCurrentTime());
        providedMesh->setMeshDimensions(2);

        auto meshKey = std::string(providedMesh->getMeshName());
        _meshes[meshKey] = std::move(providedMesh);

        // Set up loggin to a file
        Logger::getInstance().setLogFile("Solid.log");
    }


    void SolidParticipantImplementation::initialize()
    {
        // 1. Preprocess any meshes
        MINIMALCOUPLER_INFO("Starting initialization...");
        
        //2. Setup communication between Fluid and solid participants
        fluidSocket = getFluidConnectionSocket();
        MINIMALCOUPLER_INFO("Connected to Fluid!");

        //3. Transfer vertices from participant sender to receiver
        sendMeshVertices();

        //4. Map Write Data (Not needed here as it does not receive any meshes)

        //5. Initialize the coupling scheme
        getCouplingScheme().initialize(getParticipantName(), _meshes.at("Solid-Mesh").get(), fluidSocket);

        //6. Map Read Data (Not needed here as it does not receive any meshes)
        MINIMALCOUPLER_INFO("Initialization complete!");

    }

    int SolidParticipantImplementation::getFluidConnectionSocket() const
    {
        // Solid socket acts like a client, so it keeps retrying after 1s to connect to the Flui(Server)
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family  = AF_INET;
        serverAddress.sin_port = htons(5001);
        serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  

        // Retry mechanism
        bool connected = false;
        while (!connected)
        {
            if (connect(sock, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == 0)
            {
                connected = true;
            }
            else
            {
                MINIMALCOUPLER_INFO("Connection to Fluid failed, retrying in 1 second...");
                sleep(1); 
            }
        }
                
        return sock;
    }

    void SolidParticipantImplementation::sendMeshVertices() const
    {
        // function sends mesh vertices to Fluid participant
        std::string meshName = getParticipantName() + "-Mesh";

        size_t size = _meshes.at(meshName)->getVertexCount();
        MINIMALCOUPLER_INFO("Sending ", size, " vertices");
        send(fluidSocket, &size, sizeof(size), 0);

        if (size > 0)
        {
            auto vertices = _meshes.at(meshName)->getMeshVertices();

            MINIMALCOUPLER_FILE_INFO("Vertices being sent:");
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                MINIMALCOUPLER_FILE_INFO("Vertex ", i, ": (", vertices[i].x, ", ", vertices[i].y, ")");
            }

            send(fluidSocket, vertices.data(), size * sizeof(Point), 0);
        }

    }

    int SolidParticipantImplementation::getMeshDimensions(
        precice::string_view meshName) const
    {
        return _meshes.at(std::string(meshName))->getMeshDimensions();
    }

    void SolidParticipantImplementation::setMeshVertices(
        precice::string_view meshName,
        precice::span<const double> coordinates,
        precice::span<int> ids)
    {
        // recieves flat point vector from user and contructs a vector of points for the participants mesh
        auto& mesh = _meshes[std::string(meshName)];
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
        mesh->allocateDataFields();  // Allocates dimensions * vertexCount for each data field
    }

    // Data exchange methods
    void SolidParticipantImplementation::readData(
        precice::string_view meshName,
        precice::string_view dataName,
        precice::span<const precice::VertexID> vertexIDs,
        double relativeReadTime,
        precice::span<double> values) const
    {
        if (!_meshes.contains(std::string(meshName)))
        {
            throw std::runtime_error("Mesh with name " + std::string(meshName) + " not found");
        }
        auto& mesh = _meshes.at(std::string(meshName));

        if (!mesh->checkIfDataFieldExists(std::string(dataName)))
        {
            throw std::runtime_error("Data field with name " + std::string(dataName) + " not found");
        }

        for (auto id : vertexIDs)
        {
            if (!mesh->checkIfVertexIdExists(id))
            {
                throw std::runtime_error("Vertex with id " + std::to_string(id) + " does not exist");
            }
        }

        if (vertexIDs.size() * mesh->getMeshDimensions() != values.size())
        {
            throw std::runtime_error("The value array provided is not enough to store all the data values");
        }

        double absoluteTime = getCouplingScheme().getCurrentTime() + relativeReadTime;

        MINIMALCOUPLER_INFO("Reading data '", dataName, "' from mesh '", meshName, "' for ", vertexIDs.size(), " vertices at time ", absoluteTime);
        mesh->getDataForVertexId(dataName, vertexIDs, values, absoluteTime);
    }

    void SolidParticipantImplementation::writeData(
        precice::string_view meshName,
        precice::string_view dataName,
        precice::span<const precice::VertexID> vertexIDs,
        precice::span<const double> values)
    {
        MINIMALCOUPLER_INFO("Writing data '", dataName, "' to mesh '", meshName, "' for ", vertexIDs.size(), " vertices");

        // check if meshName exists
        if (!_meshes.contains(std::string(meshName)))
        {
            throw std::runtime_error("Mesh with name " + std::string(meshName) +  " not found");
        }
        auto & mesh = _meshes.at(std::string(meshName));
        // check if data name exists
        if (!mesh->checkIfDataFieldExists(std::string(dataName)))
        {
            throw std::runtime_error("Data field with name " + std::string(dataName) +  " not found");
        }
        // check if all the vertex locations are actually correct
        for (auto id: vertexIDs)
        {
            if (!mesh->checkIfVertexIdExists(id))
            {
                throw std::runtime_error("Vertex with id " + std::to_string(id) + " does not exist");
            }
        } 
        // check if size of value = size of vertexIds * dimensions =  size of values
        if (vertexIDs.size() * mesh->getMeshDimensions() != values.size())
        {
            throw std::runtime_error("The value error provided is not enough to store all the data values");
        }
        std::vector<double> dataToStore (values.data(), values.data() + values.size());
        // if all of these checks succeded then we just add the data to mesh for ts = end of current timewindow
        double absoluteTime = getCouplingScheme().getCurrentTime();
        // store the data into the mesh for time window
        
        mesh->addDataToMesh(std::string(dataName), absoluteTime, std::move(dataToStore));
    }

    void SolidParticipantImplementation::advance(
        double computedTimeStepSize)
    {
        double currentTime = getCouplingScheme().getCurrentTime();
        double maxTimeStep = getCouplingScheme().getMaxTimeStepSize();
        double nextTimeWindowEnd = currentTime + maxTimeStep;
        
        // add a check here to map data only if at window end
        if (currentTime + computedTimeStepSize >= currentTime + maxTimeStep)
        {
            getCouplingScheme().advance( 
                getParticipantName(),
                _meshes.at("Solid-Mesh").get(),
                computedTimeStepSize,
                fluidSocket);
        }
    }

    SolidParticipantImplementation::~SolidParticipantImplementation()
    {
        finalize();
    }

    void SolidParticipantImplementation::finalize()
    {
        if (fluidSocket >= 0)
        {
            close(fluidSocket);
            fluidSocket = -1;
        }
    }

    bool SolidParticipantImplementation::isCouplingOngoing() 
    {
        return getCouplingScheme().isCouplingOnGoing();
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

    double SolidParticipantImplementation::getMaxTimeStepSize()
    {
        return getCouplingScheme().getMaxTimeStepSize();
    }

    bool SolidParticipantImplementation::isTimeWindowComplete()
    {
        return getCouplingScheme().isTimeWindowComplete();
    }

    void SolidParticipantImplementation::startProfilingSection(
        const std::string& name)
    {

    }

    void SolidParticipantImplementation::stopLastProfilingSection()
    {

    }

}