#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "ParticipantImpl.hpp"

#include "data/Point.hpp"
#include "logger/logger.hpp"
#include "mapping/NearestNeighbor.hpp"
#include "utils/constants.hpp"

namespace MinimalCoupler
{

ParticipantImplementation::ParticipantImplementation(precice::string_view participantName,
                                                     precice::string_view configurationFileName, int solverProcessIndex,
                                                     int solverProcessSize)
    : _participantName(std::string(participantName)), _configFileName(std::string(configurationFileName)),
      _couplingScheme(), _rank(solverProcessIndex), _size(solverProcessSize)

{

    MINIMALCOUPLER_INFO("I am participant ", _participantName);
    MINIMALCOUPLER_INFO("My remote participant is ", _remoteParticipantName);

    _couplingScheme.setMaxTime(Constants::MAX_TIME);
    _couplingScheme.setTimeWindowSize(Constants::TIME_WINDOW_SIZE);
    _couplingScheme.setInitialRelaxation(Constants::INITIAL_RELAXATION);
    _couplingScheme.setMaxIterations(Constants::MAX_ITERATIONS);

    if (_participantName == "Solid")
    {
        _remoteParticipantName = "Fluid";
        _participantMeshName = _participantName + "-Mesh";
        _remoteParticipantMeshName = _remoteParticipantName + "-Mesh";
        _participantDataName = "Displacement";
        _remoteParticipantDataName = "Force";

        constructSolidParticipantMeshes();

        // Set up loggin to a file
        Logger::getInstance().setLogFile("Solid.log");
    }
    else
    {
        _remoteParticipantName = "Solid";
        _participantMeshName = _participantName + "-Mesh";
        _remoteParticipantMeshName = _remoteParticipantName + "-Mesh";
        _participantDataName = "Force";
        _remoteParticipantDataName = "Displacement";

        constructFluidParticipantMeshes();

        // setup logging into a file
        Logger::getInstance().setLogFile("Fluid.log");
    }
}

void ParticipantImplementation::constructFluidParticipantMeshes()
{

    // Fluid has two meshes one that it provdes and another that it recieves from the Solid. Both have two data vectors
    auto providedMesh = std::make_unique<Mesh>();
    providedMesh->setMeshName(_participantMeshName);
    providedMesh->addDataToMesh(_participantDataName, _couplingScheme.getCurrentWindowNumber());
    providedMesh->addDataToMesh(_remoteParticipantDataName, _couplingScheme.getCurrentWindowNumber());
    providedMesh->setMeshDimensions(Constants::MESH_DIMENSIONS);

    _meshes[std::string(providedMesh->getMeshName())] = std::move(providedMesh);

    auto receivedMesh = std::make_unique<Mesh>();
    receivedMesh->setMeshName(_remoteParticipantMeshName);
    receivedMesh->addDataToMesh(_participantDataName, _couplingScheme.getCurrentWindowNumber());
    receivedMesh->addDataToMesh(_remoteParticipantDataName, _couplingScheme.getCurrentWindowNumber());
    receivedMesh->setMeshDimensions(Constants::MESH_DIMENSIONS);

    _meshes[std::string(receivedMesh->getMeshName())] = std::move(receivedMesh);
}

ParticipantImplementation::~ParticipantImplementation()
{
    finalize();
}

void ParticipantImplementation::constructSolidParticipantMeshes()
{

    // Solid only provides one mesh with two data vectors
    auto providedMesh = std::make_unique<Mesh>();
    providedMesh->setMeshName(_participantMeshName);
    providedMesh->addDataToMesh(_participantDataName, _couplingScheme.getCurrentWindowNumber());
    providedMesh->addDataToMesh(_remoteParticipantDataName, _couplingScheme.getCurrentWindowNumber());
    providedMesh->setMeshDimensions(2);

    auto meshKey = std::string(providedMesh->getMeshName());
    _meshes[meshKey] = std::move(providedMesh);
}

void ParticipantImplementation::initialize()
{

    // 1. Preprocess any meshes (Not needed)
    MINIMALCOUPLER_INFO("Starting initialization...");

    // 2. Setup communication between Fluid and solid participants
    _remoteSocket = (_participantName == "Solid") ? getFluidConnectionSocket() : getSolidConnectionSocket();

    MINIMALCOUPLER_INFO("Connected to ", _remoteParticipantName);

    // 3. Transfer vertices from participant sender to receiver
    if (_participantName == "Solid")
    {
        sendMeshVertices();
    }
    else
    {
        receiveMeshVertices();
    }

    // compute mappings between meshes
    // The participant that receives the mesh MUST compute mappings in both the read and write directions
    computeMappings();

    // 4. Map Write Data (at time 0 for initialization)
    mapWriteData();

    // 5. Initialize the coupling scheme
    _couplingScheme.initialize(
        _participantName,
        _meshes.at((_participantName == "Solid") ? _participantMeshName : _remoteParticipantMeshName).get(),
        _remoteSocket);

    // 6. Map Read Data (data was stored at window 0 during initialize)
    mapReadData(_couplingScheme.getCurrentWindowNumber());

    MINIMALCOUPLER_INFO("Initialization complete!");
}

int ParticipantImplementation::getSolidConnectionSocket() const
{
    // Fluid acts as a server in the TCP connects. So it binds to a port and Listens for solid connection
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(Constants::SERVER_PORT);
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
int ParticipantImplementation::getFluidConnectionSocket() const
{
    // Solid socket acts like a client, so it keeps retrying after 1s to connect to the Flui(Server)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(Constants::SERVER_PORT);
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

void ParticipantImplementation::sendMeshVertices() const
{
    // function sends mesh vertices to Fluid participant
    std::string meshName = _participantMeshName;

    size_t size = _meshes.at(meshName)->getVertexCount();
    MINIMALCOUPLER_INFO("Sending ", size, " vertices");
    send(_remoteSocket, &size, sizeof(size), 0);

    if (size > 0)
    {
        auto vertices = _meshes.at(meshName)->getMeshVertices();

        MINIMALCOUPLER_FILE_INFO("Vertices being sent:");
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            MINIMALCOUPLER_FILE_INFO("Vertex ", i, ": (", vertices[i].x, ", ", vertices[i].y, ")");
        }

        send(_remoteSocket, vertices.data(), size * sizeof(Point), 0);
    }
}

int ParticipantImplementation::getMeshDimensions(precice::string_view meshName) const
{
    return _meshes.at(std::string(meshName))->getMeshDimensions();
}

void ParticipantImplementation::receiveMeshVertices() const
{
    // recieves mesh vertices from the Solid Participant
    size_t size;
    recv(_remoteSocket, &size, sizeof(size), 0);
    MINIMALCOUPLER_INFO("Receiving ", size, " vertices from Solid");

    if (size > 0)
    {
        std::vector<Point> vertices(size);
        recv(_remoteSocket, vertices.data(), size * sizeof(Point), MSG_WAITALL);

        MINIMALCOUPLER_INFO("Solid mesh vertices set.");
        _meshes.at(_remoteParticipantMeshName)->setMeshVertices(vertices);
        _meshes.at(_remoteParticipantMeshName)->allocateDataFields();
    }
}

void ParticipantImplementation::setMeshVertices(precice::string_view meshName, precice::span<const double> coordinates,
                                                precice::span<int> ids)
{

    // recieves flat point vector from user and contructs a vector of points for the participants mesh
    auto &mesh = _meshes[std::string(meshName)];
    int dim = mesh->getMeshDimensions();

    std::vector<Point> vertices;
    vertices.reserve(coordinates.size() / dim);

    for (size_t i = 0; i < coordinates.size(); i += dim)
    {
        ids[i / dim] = static_cast<precice::VertexID>(i / dim);
        Point v{ids[i / dim], coordinates[i], coordinates[i + 1]};
        vertices.emplace_back(std::move(v));
    }

    mesh->setMeshVertices(vertices);
    mesh->allocateDataFields(); // Allocates dimensions * vertexCount for each data field
}

void ParticipantImplementation::readData(precice::string_view meshName, precice::string_view dataName,
                                         precice::span<const precice::VertexID> vertexIDs, double relativeReadTime,
                                         precice::span<double> values) const
{

    if (!_meshes.contains(std::string(meshName)))
    {
        throw std::runtime_error("Mesh with name " + std::string(meshName) + " not found");
    }
    auto &mesh = _meshes.at(std::string(meshName));

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

    int window = _couplingScheme.getCurrentWindowNumber();

    // After the final advance, the window number may point to a window that was never populated.
    // Fall back to the latest available window so the solver can still read final results.
    if (!mesh->checkIfTimeWindowExists(std::string(dataName), window))
    {
        auto available = mesh->getAvailableTimeWindows(std::string(dataName));
        if (available.empty())
        {
            throw std::runtime_error("No data available for field " + std::string(dataName));
        }
        window = available.back();
        MINIMALCOUPLER_INFO("Reading data '", dataName, "': window ", _couplingScheme.getCurrentWindowNumber(),
                            " not available, falling back to window ", window);
    }

    MINIMALCOUPLER_INFO("Reading data '", dataName, "' from mesh '", meshName, "' for ", vertexIDs.size(),
                        " vertices at window ", window);
    mesh->getDataForVertexId(dataName, vertexIDs, values, window);
}

void ParticipantImplementation::writeData(precice::string_view meshName, precice::string_view dataName,
                                          precice::span<const precice::VertexID> vertexIDs,
                                          precice::span<const double> values)
{

    MINIMALCOUPLER_INFO("Writing data '", dataName, "' to mesh '", meshName, "' for ", vertexIDs.size(), " vertices");

    // check if meshName exists
    if (!_meshes.contains(std::string(meshName)))
    {
        throw std::runtime_error("Mesh with name " + std::string(meshName) + " not found");
    }
    auto &mesh = _meshes.at(std::string(meshName));
    // check if data name exists
    if (!mesh->checkIfDataFieldExists(std::string(dataName)))
    {
        throw std::runtime_error("Data field with name " + std::string(dataName) + " not found");
    }
    // check if all the vertex locations are actually correct
    for (auto id : vertexIDs)
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
    std::vector<double> dataToStore(values.data(), values.data() + values.size());
    int window = _couplingScheme.getCurrentWindowNumber();

    mesh->addDataToMesh(std::string(dataName), window, std::move(dataToStore));
}

void ParticipantImplementation::finalize()
{
    if (_remoteSocket >= 0)
    {
        close(_remoteSocket);
        _remoteSocket = -1;
    }
}

void ParticipantImplementation::computeMappings()
{
    // No mappings needed for solid
    if (_participantName == "Solid")
    {
        return;
    }

    // Here we compute vertex mapping from Solid -> FLuid mesh and Fluid-> Solid. This will help in further read and
    // write data mappings
    MINIMALCOUPLER_INFO("Computing mappings between Fluid and Solid meshes vertices");
    NearestNeighbor nnMapper;

    const auto &fluidMeshVertices = _meshes.at(_participantMeshName)->getMeshVertices();
    const auto &solidMeshVertices = _meshes.at(_remoteParticipantMeshName)->getMeshVertices();

    auto fluidToSolidMapping = nnMapper.computeNearestNeighbors(solidMeshVertices, fluidMeshVertices);

    MINIMALCOUPLER_FILE_INFO("Fluid to Solid Mapping (write):");
    for (const auto &m : fluidToSolidMapping)
    {
        MINIMALCOUPLER_FILE_INFO("Fluid Vertex: (", fluidMeshVertices[m.id].x, ",", fluidMeshVertices[m.id].y,
                                 ") maps to Solid Vertex ", m.id, ": (", m.x, ", ", m.y, ")");
    }

    MINIMALCOUPLER_INFO("Computing mappings between Solid and Fluid meshes vertices");
    auto solidToFluidMapping = nnMapper.computeNearestNeighbors(fluidMeshVertices, solidMeshVertices);

    MINIMALCOUPLER_FILE_INFO("Solid to Fluid Mapping (read):");
    for (const auto &m : solidToFluidMapping)
    {
        MINIMALCOUPLER_FILE_INFO("Solid Vertex: (", solidMeshVertices[m.id].x, ",", solidMeshVertices[m.id].y,
                                 ") maps to Fluid Vertex ", m.id, ": (", m.x, ", ", m.y, ")");
    }

    _meshes.at(_participantMeshName)->setWriteMapping(std::move(fluidToSolidMapping));
    _meshes.at(_participantMeshName)->setReadMapping(std::move(solidToFluidMapping));
}

void ParticipantImplementation::mapWriteData()
{

    // No mappings needed for solid
    if (_participantName == "Solid")
    {
        return;
    }

    int window = _couplingScheme.getCurrentWindowNumber();
    // Before sending force data to solid, the data is mapped from FLuid mesh to solid mesh
    MINIMALCOUPLER_INFO("Trying to get Force at window: ", window, " for write mapping.");
    const auto &fluidForceData = _meshes.at(_participantMeshName)->getDataField(_participantDataName, window);

    // since we are mapping now the window will not exist on the receiving map, so we add it here
    _meshes.at(_remoteParticipantMeshName)->addDataToMesh(_participantDataName, window);

    auto &solidForceData = _meshes.at(_remoteParticipantMeshName)->getDataField(_participantDataName, window);

    NearestNeighbor::mapConservative(_meshes.at(_participantMeshName)->getWriteMapping(), fluidForceData,
                                     solidForceData, _meshes.at(_participantMeshName)->getMeshDimensions());

    MINIMALCOUPLER_FILE_INFO("Mapping force data from Fluid to Solid");
    int dim = _meshes.at(_remoteParticipantMeshName)->getMeshDimensions();
    for (size_t i = 0; i < solidForceData.size(); i += dim)
    {
        MINIMALCOUPLER_FILE_INFO("Vertex ", i / dim, ": (", solidForceData[i], ", ", solidForceData[i + 1], ")");
    }
}

void ParticipantImplementation::mapReadData(int sourceWindow)
{
    // No mappings needed for solid
    if (_participantName == "Solid")
    {
        return;
    }

    // After receiving displacement data from solid, disp data is mapped from Solid mesh to Fluid mesh
    int destWindow = _couplingScheme.getCurrentWindowNumber();

    MINIMALCOUPLER_INFO("Mapping Displacement from source window ", sourceWindow, " to dest window ", destWindow);
    _meshes.at(_participantMeshName)->addDataToMesh(_remoteParticipantDataName, destWindow);

    auto &fluidDispData = _meshes.at(_participantMeshName)->getDataField(_remoteParticipantDataName, destWindow);

    const auto &solidDispData =
        _meshes.at(_remoteParticipantMeshName)->getDataField(_remoteParticipantDataName, sourceWindow);

    NearestNeighbor::mapConsistent(_meshes.at(_participantMeshName)->getReadMapping(), solidDispData, fluidDispData,
                                   _meshes.at(_participantMeshName)->getMeshDimensions());

    MINIMALCOUPLER_FILE_INFO("Mapping displacement data from Solid mesh to Fluid mesh");
    int dim = _meshes.at(_participantMeshName)->getMeshDimensions();
    for (size_t i = 0; i < fluidDispData.size(); i += dim)
    {
        MINIMALCOUPLER_FILE_INFO("Vertex ", i / dim, ": (", fluidDispData[i], ", ", fluidDispData[i + 1], ")");
    }
}

void ParticipantImplementation::advance(double computedTimeStepSize)
{

    double currentTime = _couplingScheme.getCurrentTime();
    double maxTimeStep = _couplingScheme.getMaxTimeStepSize();

    // add a check here to map data only if at window end
    if (currentTime + computedTimeStepSize >= currentTime + maxTimeStep)
    {
        // Capture window number before exchange (coupling may increment it on convergence)
        int windowBeforeAdvance = _couplingScheme.getCurrentWindowNumber();

        // map write data first
        mapWriteData();

        // exchange data
        _couplingScheme.advance(
            _participantName,
            _meshes.at((_participantName == "Solid") ? _participantMeshName : _remoteParticipantMeshName).get(),
            computedTimeStepSize, _remoteSocket);

        // map data that was received
        if (isCouplingOngoing())
            mapReadData(windowBeforeAdvance);
    }
}

bool ParticipantImplementation::isCouplingOngoing()
{
    return _couplingScheme.isCouplingOnGoing();
}

bool ParticipantImplementation::requiresInitialData() const
{
    return false;
}

bool ParticipantImplementation::requiresWritingCheckpoint()
{
    return _couplingScheme.requiresWritingCheckpoing();
}

bool ParticipantImplementation::requiresReadingCheckpoint() 
{
    return _couplingScheme.requiresReadingCheckpoing();
}

double ParticipantImplementation::getMaxTimeStepSize()
{
    return _couplingScheme.getMaxTimeStepSize();
}

bool ParticipantImplementation::isTimeWindowComplete()
{
    return _couplingScheme.isTimeWindowComplete();
}

void ParticipantImplementation::startProfilingSection(const std::string &name)
{
}

void ParticipantImplementation::stopLastProfilingSection()
{
}
} // namespace MinimalCoupler