#include <arpa/inet.h>
#include <iomanip>
#include <math.h>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "ParticipantImpl.hpp"

#include "Utils.hpp"
#include "constants.hpp"
#include "logger.hpp"
#include <algorithm>

namespace MinimalCoupler {

ParticipantImplementation::ParticipantImplementation(
    precice::string_view participantName,
    precice::string_view configurationFileName,
    int                  solverProcessIndex,
    int                  solverProcessSize
)
    : _participantName(std::string(participantName))
    , _configFileName(std::string(configurationFileName))
    , _rank(solverProcessIndex)
    , _size(solverProcessSize)
    , _maxTime(Constants::MAX_TIME)
    , _timeWindowSize(Constants::TIME_WINDOW_SIZE)
    , _currentTimeWindowNumber(0)
    , _requiresWritingCheckPoint(false)
    , _requiresReadingCheckPoint(false)
    , _timeWindowComplete(false)
    , _watchpointVertexIndex(-1)

{
    if (_participantName == "Solid") {
        _remoteParticipantName     = "Fluid";
        _participantMeshName       = _participantName + "-Mesh";
        _remoteParticipantMeshName = _remoteParticipantName + "-Mesh";
        _participantDataName       = "Displacement";
        _remoteParticipantDataName = "Force";

        constructSolidParticipantMeshes();
    } else {
        _remoteParticipantName     = "Solid";
        _participantMeshName       = _participantName + "-Mesh";
        _remoteParticipantMeshName = _remoteParticipantName + "-Mesh";
        _participantDataName       = "Force";
        _remoteParticipantDataName = "Displacement";

        constructFluidParticipantMeshes();
    }
    MINIMALCOUPLER_INFO("I am participant ", _participantName);
    MINIMALCOUPLER_INFO("My remote participant is ", _remoteParticipantName);
}

/*
Function constructs two meshes provided and received with two data vectors for Fluid and Solid
*/
void ParticipantImplementation::constructFluidParticipantMeshes()
{
    // Fluid has two meshes one that it provdes and another that it recieves from the Solid. Both have two data vectors
    _providedMesh.setMeshName(_participantMeshName);
    _providedMesh.addDataToMesh(_participantDataName, _currentTimeWindowNumber);
    _providedMesh.addDataToMesh(_remoteParticipantDataName, _currentTimeWindowNumber);
    _providedMesh.setMeshDimensions(Constants::MESH_DIMENSIONS);

    _receivedMesh.setMeshName(_remoteParticipantMeshName);
    _receivedMesh.addDataToMesh(_participantDataName, _currentTimeWindowNumber);
    _receivedMesh.addDataToMesh(_remoteParticipantDataName, _currentTimeWindowNumber);
    _receivedMesh.setMeshDimensions(Constants::MESH_DIMENSIONS);
}

ParticipantImplementation::~ParticipantImplementation()
{
    finalize();
}

/*
Function constructs a mesh with two data vectors for Fluid and Solid
*/
void ParticipantImplementation::constructSolidParticipantMeshes()
{
    // Solid only provides one mesh with two data vectors
    _providedMesh.setMeshName(_participantMeshName);
    _providedMesh.addDataToMesh(_participantDataName, _currentTimeWindowNumber);
    _providedMesh.addDataToMesh(_remoteParticipantDataName, _currentTimeWindowNumber);
    _providedMesh.setMeshDimensions(Constants::MESH_DIMENSIONS);
}

/*
Function initializes the participant, sets up communication between participants, exchanges mesh vertices and initializes the coupling
*/
void ParticipantImplementation::initialize()
{
    MINIMALCOUPLER_INFO("Starting initialization...");

    // Setup communication between Fluid and solid participants
    _remoteSocket = (_participantName == "Solid") ? getFluidConnectionSocket() : getSolidConnectionSocket();

    MINIMALCOUPLER_INFO("Connected to ", _remoteParticipantName);

    // Transfer vertices from participant sender to receiver
    if (_participantName == "Solid") {
        sendMeshVertices();
    } else {
        receiveMeshVertices();
        _receivedMesh.addDataToMesh(_participantDataName, _currentTimeWindowNumber);
        _receivedMesh.addDataToMesh(_remoteParticipantDataName, _currentTimeWindowNumber);
    }

    // The participant that receives the mesh MUST compute mappings in both the read and write directions
    computeMappings();

    // Map Write Data (at time 0 for initialization)
    if (_participantName == "Fluid")
    {
        mapWriteData();
    }

    // Initialize coupling scheme
    couplingSchemeInitialize();

    // Map Read Data (data was stored at window 0 during initialize)
    if (_participantName == "Fluid")
    {
        mapReadData(_currentTimeWindowNumber);
    }

    MINIMALCOUPLER_INFO("Initialization complete!");
}


int ParticipantImplementation::getSolidConnectionSocket() const
{
    // Fluid acts as a server in the TCP connects. So it binds to a port and Listens for solid connection
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_port        = htons(Constants::SERVER_PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Convert to network byte order

    if (bind(sock, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        close(sock);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(sock, 1) < 0) {
        close(sock);
        throw std::runtime_error("Unable to listen on socket");
    }

    int client = accept(sock, nullptr, nullptr);
    if (client < 0) {
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
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_port        = htons(Constants::SERVER_PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // Retry mechanism
    bool connected = false;
    while (!connected) {
        if (connect(sock, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == 0) {
            connected = true;
        } else {
            MINIMALCOUPLER_INFO("Connection to Fluid failed, retrying in 1 second...");
            sleep(1);
        }
    }

    return sock;
}

/*
Function is called by Solid to send vertices to Fluid
*/
void ParticipantImplementation::sendMeshVertices() const
{
    // function sends mesh vertices to Fluid participant
    std::string meshName = _participantMeshName;

    Utils::sendPoints(_remoteSocket, _providedMesh.getMeshVertices());
    size_t size = _providedMesh.getVertexCount();
    MINIMALCOUPLER_INFO("Sending ", size, " vertices");
}

int ParticipantImplementation::getMeshDimensions(
    precice::string_view meshName) const
{
    return (
        std::string(_providedMesh.getMeshName()) ==
        std::string(meshName)) ? 
        _providedMesh.getMeshDimensions() : 
        _receivedMesh.getMeshDimensions();
}

int ParticipantImplementation::getDataDimensions(
    precice::string_view meshName,
    precice::string_view dataName) const
{
    return (
        std::string(_providedMesh.getMeshName()) == 
        std::string(meshName)) ? 
        _providedMesh.getMeshDimensions() :
        _receivedMesh.getMeshDimensions();
}

/*
Function is called by Fluid to recv vertices from Solid
*/
void ParticipantImplementation::receiveMeshVertices()
{
    // recieves mesh vertices from the Solid Participant
    std::vector<Point> data;
    Utils::recvPoints(_remoteSocket, data);
    _receivedMesh.setMeshVertices(data);
    MINIMALCOUPLER_INFO("Recieved ", _receivedMesh.getVertexCount(), " vertices from Solid");
}

/*
Function recv mesh vertices from the user and sets these vertices to provided mesh and gives back to
the use the vertex ID
*/
void ParticipantImplementation::setMeshVertices(
    precice::string_view        meshName,
    precice::span<const double> coordinates,
    precice::span<int>          ids
)
{
    // recieves flat point vector from user and contructs a vector of points for the participants mesh
    if (std::string(meshName) != std::string(_providedMesh.getMeshName())) {
        throw std::runtime_error("Setting vertices for received mesh is not allowed");
        return;
    }

    int dim = _providedMesh.getMeshDimensions();

    std::vector<Point> vertices;
    vertices.reserve(coordinates.size() / dim);

    // Math to store vertices because vertieces is a vector of Points
    for (size_t i = 0; i < coordinates.size(); i += dim) {
        ids[i / dim] = static_cast<precice::VertexID>(i / dim);
        Point v { ids[i / dim], coordinates[i], coordinates[i + 1] };
        vertices.emplace_back(std::move(v));
    }

    _providedMesh.setMeshVertices(vertices);
    _providedMesh.addDataToMesh(_participantDataName, _currentTimeWindowNumber);
    _providedMesh.addDataToMesh(_remoteParticipantDataName, _currentTimeWindowNumber);

    // Find the index of the point closest to watchpoint
    if (meshName == "Solid-Mesh") {
        setupWatchPoint();
    }
}

/*
Function finds the index of the point closest to the watchpoint
*/
void ParticipantImplementation::setupWatchPoint()
{

        Point       watchPoint { -1, Constants::WATCHPOINT_X, Constants::WATCHPOINT_Y };
        Point       closestPoint;
        double      bestDist = std::numeric_limits<double>::max();
        const auto& vertices    = _providedMesh.getMeshVertices();
        for (const auto& v : vertices) {
            double dist = Utils::euclideanDistance(watchPoint, v);
            if (dist < bestDist) {
                bestDist     = dist;
                closestPoint = v;
            }
        }
        _watchpointVertexIndex = closestPoint.id;
}

void ParticipantImplementation::readData(
    precice::string_view                   meshName,
    precice::string_view                   dataName,
    precice::span<const precice::VertexID> vertexIDs,
    double                                 relativeReadTime,
    precice::span<double>                  values
) const
{
    if (std::string(meshName) != std::string(_providedMesh.getMeshName())) {
        throw std::runtime_error("Mesh with name " + std::string(meshName) + " not found");
    }

    if (!_providedMesh.checkIfDataFieldExists(std::string(dataName))) {
        throw std::runtime_error("Data field with name " + std::string(dataName) + " not found");
    }

    for (auto id : vertexIDs) {
        if (!_providedMesh.checkIfVertexIdExists(id)) {
            throw std::runtime_error("Vertex with id " + std::to_string(id) + " does not exist");
        }
    }

    int window = _currentTimeWindowNumber;

    // After the final advance, the window number may point to a window that was never populated.
    if (!_providedMesh.checkIfTimeWindowExists(std::string(dataName), window)) {
        auto available = _providedMesh.getAvailableTimeWindows(std::string(dataName));
        if (available.empty()) {
            throw std::runtime_error("No data available for field " + std::string(dataName));
        }

        // Fall back to the latest available window so the solver can still read final results.
        window = available.back();
    }

    MINIMALCOUPLER_INFO(
        "Reading data '",
        dataName,
        "' from mesh '",
        meshName,
        "' for ",
        vertexIDs.size(),
        " vertices at window ",
        window
    );
    _providedMesh.getDataForVertexId(dataName, vertexIDs, values, window);
}

void ParticipantImplementation::writeData(
    precice::string_view                   meshName,
    precice::string_view                   dataName,
    precice::span<const precice::VertexID> vertexIDs,
    precice::span<const double>            values
)
{
    if (std::string(meshName) != std::string(_providedMesh.getMeshName())) {
        throw std::runtime_error("Mesh with name " + std::string(meshName) + " not found");
    }

    MINIMALCOUPLER_INFO("Writing data '", dataName, "' to mesh '", meshName, "' for ", vertexIDs.size(), " vertices");

    // check if data name exists
    if (!_providedMesh.checkIfDataFieldExists(std::string(dataName))) {
        throw std::runtime_error("Data field with name " + std::string(dataName) + " not found");
    }

    for (auto id : vertexIDs) {
        if (!_providedMesh.checkIfVertexIdExists(id)) {
            throw std::runtime_error("Vertex with id " + std::to_string(id) + " does not exist");
        }
    }

    std::vector<double> dataToStore(
        values.data(),
        values.data() + values.size());

    _providedMesh.addDataToMesh(
        std::string(dataName),
        _currentTimeWindowNumber,
        std::move(dataToStore));
}

void ParticipantImplementation::finalize()
{
    if (_remoteSocket >= 0) {
        close(_remoteSocket);
        _remoteSocket = -1;
    }
}

/*
Function is a wrapper that is called to map each Fluid vertex to a solid Vertex
*/
void ParticipantImplementation::computeMappings()
{
    // No mappings needed for solid
    if (_participantName == "Solid") {
        return;
    }

    // Here we compute vertex mapping from Fluid -> Solid mesh .This will help in further read and
    // write data mappings
    MINIMALCOUPLER_INFO("Computing mappings between Fluid and Solid meshes vertices");

    const auto& fluidMeshVertices = _providedMesh.getMeshVertices();
    const auto& solidMeshVertices = _receivedMesh.getMeshVertices();

    // actual NN computaion takes place here
    computeNearestNeighbors(fluidMeshVertices, solidMeshVertices);
}

/*
Functions mapps data from fluid to solid participant conservatively
*/
void ParticipantImplementation::mapWriteData()
{
    MINIMALCOUPLER_INFO("Retreive to get Force at window: ", _currentTimeWindowNumber, " for write mapping.");

    auto& fluidForceData = _providedMesh.getDataField(
        _participantDataName, 
        _currentTimeWindowNumber);

    // since we are mapping now the window will not exist on the receiving map, so we add it here
    _receivedMesh.addDataToMesh(
        _participantDataName,
        _currentTimeWindowNumber);

    auto  dimensions     = _providedMesh.getMeshDimensions();
    auto& solidForceData = _receivedMesh.getDataField(
        _participantDataName,
        _currentTimeWindowNumber);

    // for every force data in fluid vertex and each mapping I sum up the fluid force data from the fluid mesh
    // and store it as force data in the solid mesh
    std::ranges::fill(solidForceData, 0.0);
    for (size_t i = 0; i < _vertexMapping.size(); ++i) {
        int targetVertexID = _vertexMapping[i].id;
        for (int d = 0; d < dimensions; ++d) {
            solidForceData[targetVertexID * dimensions + d] += fluidForceData[i * dimensions + d];
        }
    }
}

/*
Functions mapps data from solid to fluid participant consistently
*/
void ParticipantImplementation::mapReadData(int sourceWindow)
{

    int destWindow = _currentTimeWindowNumber;

    MINIMALCOUPLER_INFO("Mapping Displacement from source window ", sourceWindow, " to dest window ", destWindow);
    auto& solidDispData = _receivedMesh.getDataField(_remoteParticipantDataName, sourceWindow);

    // since we are mapping now the window will not exist on the receiving map, so we add it here
    _providedMesh.addDataToMesh(_remoteParticipantDataName, destWindow);

    auto& fluidDispData = _providedMesh.getDataField(_remoteParticipantDataName, destWindow);

    auto dimensions = _providedMesh.getMeshDimensions();
    fluidDispData.resize(_vertexMapping.size() * dimensions);
    std::ranges::fill(fluidDispData, 0.0);

    // for every disp data in solid vertex and each mapping I store it as it is in the fluid's solid mesh
    for (size_t i = 0; i < _vertexMapping.size(); ++i) {
        int sourceVertexID = _vertexMapping[i].id;
        for (int d = 0; d < dimensions; ++d) {
            fluidDispData[i * dimensions + d] = solidDispData[sourceVertexID * dimensions + d];
        }
    }
}

void ParticipantImplementation::advance(double computedTimeStepSize)
{
    _timeWindowComplete = false;
    double currentTime = getCurrentTime();

    double windowEnd = _timeWindowSize * (_currentTimeWindowNumber + 1);
    if (currentTime + computedTimeStepSize >= windowEnd - Constants::DOUBLE_EQUALITY_BOUND) {
        // Capture window number before exchange (coupling may increment it on convergence)
        int windowBeforeAdvance = _currentTimeWindowNumber;

        // map write data first
        if (_participantName == "Fluid")
        {
            mapWriteData();
        }

        // exchange data
        couplingSchemeAdvance();

        // map data that was received
        if (isCouplingOngoing() && _participantName == "Fluid")
        {
            mapReadData(windowBeforeAdvance);
        }
    }
}

bool ParticipantImplementation::isCouplingOngoing()
{
    return getCurrentTime() < _maxTime;
}

bool ParticipantImplementation::requiresInitialData()
{
    return _providedMesh.requiresInitialData();
}

bool ParticipantImplementation::requiresWritingCheckpoint()
{
    bool res = _requiresWritingCheckPoint;
    if (_requiresWritingCheckPoint) {
        _requiresWritingCheckPoint = false;
    }
    return res;
}

bool ParticipantImplementation::requiresReadingCheckpoint()
{
    bool res = _requiresReadingCheckPoint;
    if (_requiresReadingCheckPoint) {
        _requiresReadingCheckPoint = false;
    }
    return res;
}

double ParticipantImplementation::getMaxTimeStepSize()
{
    // Returns time until the next window end
    return _timeWindowSize * (_currentTimeWindowNumber + 1) - getCurrentTime();
}

bool ParticipantImplementation::isTimeWindowComplete()
{
    return _timeWindowComplete;
}

/*
Function computes the nearest neighbors for each point in queryPoints to each point in searchSpaceOfPoints
and sets the _vertexMapping variable.
*/
void ParticipantImplementation::computeNearestNeighbors(
    const std::vector<Point>& queryPoints,
    const std::vector<Point>& searchSpaceOfPoints
)
{
    _vertexMapping.resize(queryPoints.size());

    for (size_t i = 0; i < queryPoints.size(); ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (size_t j = 0; j < searchSpaceOfPoints.size(); ++j) {
            double dist = Utils::euclideanDistance(queryPoints[i], searchSpaceOfPoints[j]);
            if (dist < bestDist) {
                bestDist          = dist;
                _vertexMapping[i] = searchSpaceOfPoints[j];
            }
        }
    }
}

double ParticipantImplementation::getCurrentTime() const
{
    return _timeWindowSize * _currentTimeWindowNumber;
}

void ParticipantImplementation::openConvergenceLog()
{
    _convergenceLog.open("convergence.log", std::ios::out | std::ios::trunc);
    _convergenceLog << "TimeWindow\tIteration\tResRel\tConverged" << std::endl;

    _watchpointLog.open("watchpoint-Flap-Tip.log", std::ios::out | std::ios::trunc);
    _watchpointLog << "  Time  Coordinate0  Coordinate1  Displacement0  Displacement1  Force0  Force1" << std::endl;
}

void ParticipantImplementation::writeConvergenceEntry()
{
    if (!_convergenceLog.is_open())
        return;

    int iterNum = _aitken.isConverged() ? _aitken.getIterationNumber() + 1 : _aitken.getIterationNumber();
    _convergenceLog << (_currentTimeWindowNumber + 1) << "\t" << iterNum << "\t"
                    << std::scientific << std::setprecision(6) << _aitken.getResidualForLog() << "\t"
                    << (_aitken.isConverged() ? 1 : 0) << std::endl;
}

void ParticipantImplementation::writeWatchpointEntry(double time, int window)
{
    if (!_watchpointLog.is_open() || _watchpointVertexIndex < 0)
        return;

    const auto& vertices = _providedMesh.getMeshVertices();
    int         vidx     = _watchpointVertexIndex;

    const auto& relaxedData = _aitken.getRelaxedData();
    double      disp0       = relaxedData[vidx * 2];
    double      disp1       = relaxedData[vidx * 2 + 1];

    const auto& forceData = _providedMesh.getDataField("Force", window);
    double      force0    = forceData[vidx * 2];
    double      force1    = forceData[vidx * 2 + 1];

    _watchpointLog << " " << std::scientific << std::setprecision(8) << time << "  " << vertices[vidx].x << "   "
                   << vertices[vidx].y << "   " << disp0 << "   " << disp1 << "   " << force0 << "  " << force1
                   << std::endl;
}

void ParticipantImplementation::couplingSchemeInitialize()
{
    MINIMALCOUPLER_INFO("Current window = ", _currentTimeWindowNumber);

    if (_participantName == "Fluid") {
        MINIMALCOUPLER_INFO("Retrieving Force data at window ", _currentTimeWindowNumber);
        const auto& fluidData = _receivedMesh.getDataField("Force", _currentTimeWindowNumber);

        MINIMALCOUPLER_INFO("Sending ", fluidData.size(), " initial force values to Solid");
        Utils::sendVector(_remoteSocket, fluidData);

        std::vector<double> dispData;
        Utils::recvVector(_remoteSocket, dispData);
        MINIMALCOUPLER_INFO("Received ", dispData.size(), " initial displacement values from Solid");

        _receivedMesh.addDataToMesh("Displacement", _currentTimeWindowNumber, std::move(dispData));
        MINIMALCOUPLER_INFO("Initialize complete");
    }

    if (_participantName == "Solid") {
        std::vector<double> forceData;
        Utils::recvVector(_remoteSocket, forceData);
        MINIMALCOUPLER_INFO("Received ", forceData.size(), " initial force values from Fluid");

        _providedMesh.addDataToMesh(_remoteParticipantDataName, _currentTimeWindowNumber, std::move(forceData));

        const auto& solidData = _providedMesh.getDataField(_participantDataName, _currentTimeWindowNumber);
        MINIMALCOUPLER_INFO("Sending ", solidData.size(), " initial displacement values to Fluid");
        Utils::sendVector(_remoteSocket, solidData);

        // Initialize _data for Aitken relaxation with current displacement as first guess
        _aitken.setRelaxedData(solidData);

        openConvergenceLog();
        writeWatchpointEntry(0.0, _currentTimeWindowNumber);
    }

    // Solid blocks waiting for first Force from Fluid's advance()
    if (_participantName == "Solid") {
        MINIMALCOUPLER_INFO("Blocking, waiting for Force from Fluid's first advance()...");

        std::vector<double> forceData;
        Utils::recvVector(_remoteSocket, forceData);

        // This Force is for solving window 0, so store at current window
        MINIMALCOUPLER_INFO("Received ", forceData.size(), " force values for window ", _currentTimeWindowNumber);
        MINIMALCOUPLER_INFO(
            "FORCE RECV DEBUG (init block2): first values = ",
            (!forceData.empty() ? forceData[0] : 0.0),
            ", ",
            (forceData.size() > 1 ? forceData[1] : 0.0)
        );
        _providedMesh.addDataToMesh(_remoteParticipantDataName, _currentTimeWindowNumber, std::move(forceData));
        // Verify stored correctly
        const auto& storedForce = _providedMesh.getDataField(_remoteParticipantDataName, _currentTimeWindowNumber);
        MINIMALCOUPLER_INFO(
            "FORCE VERIFY (init block2): stored[0]=",
            storedForce[0],
            " stored[1]=",
            storedForce[1],
            " size=",
            storedForce.size()
        );
        MINIMALCOUPLER_INFO("Initialize complete, ready to run solver");
    }

    _requiresWritingCheckPoint = true;
}

void ParticipantImplementation::couplingSchemeAdvance()
{
    if (_participantName == "Fluid") {
        MINIMALCOUPLER_INFO("Advance with window number", _currentTimeWindowNumber);

        // Validate Force data exists at current window
        if (!_receivedMesh.checkIfTimeWindowExists("Force", _currentTimeWindowNumber)) {
            MINIMALCOUPLER_INFO("Force data does not exist at window ", _currentTimeWindowNumber, "!");
            return;
        }

        // Send Force for current iteration
        const auto& fluidData = _receivedMesh.getDataField("Force", _currentTimeWindowNumber);
        MINIMALCOUPLER_INFO("Sending ", fluidData.size(), " force values to Solid");
        Utils::sendVector(_remoteSocket, fluidData);

        // Block waiting for Displacement + convergence from Solid
        MINIMALCOUPLER_INFO("Waiting for Displacement data and convergence data from Solid...");
        std::vector<double> dispData;
        Utils::recvVector(_remoteSocket, dispData);
        bool converged;
        Utils::recvBool(_remoteSocket, converged);

        _receivedMesh.addDataToMesh("Displacement", _currentTimeWindowNumber, std::move(dispData));

        if (converged) {
            _currentTimeWindowNumber++;
            _timeWindowComplete = true;
            MINIMALCOUPLER_INFO("Window converged, Hence Advanced to window ", _currentTimeWindowNumber);
            _requiresWritingCheckPoint = true;
        } else {
            _requiresReadingCheckPoint = true;
            MINIMALCOUPLER_INFO(" Window has not converged");
        }
    }

    if (_participantName == "Solid") {
        MINIMALCOUPLER_INFO("Advance with window number", _currentTimeWindowNumber);

        // Validate Displacement data exists at current window
        if (!_providedMesh.checkIfTimeWindowExists("Displacement", _currentTimeWindowNumber)) {
            MINIMALCOUPLER_INFO("ERROR: Displacement data does not exist at window ", _currentTimeWindowNumber, "!");
            return;
        }

        const auto& rawDispData = _providedMesh.getDataField("Displacement", _currentTimeWindowNumber);
        MINIMALCOUPLER_INFO("Solid: Got raw displacement (", rawDispData.size(), " values) from solver");

        // Apply Aitken relaxation and check convergence
        std::vector<double> rawDispVec(rawDispData.begin(), rawDispData.end());
        _aitken.computeAitkenRelaxedOutput(rawDispVec);

        writeConvergenceEntry();

        const auto& relaxedData = _aitken.getRelaxedData();
        Utils::sendVector(_remoteSocket, relaxedData);

        MINIMALCOUPLER_INFO("Sending ", relaxedData.size(), " relaxed displacement values to Fluid");

        bool converged = _aitken.isConverged();
        Utils::sendBool(_remoteSocket, converged);

        MINIMALCOUPLER_INFO("Sending convergence status: ", converged);
        if (converged) {
            int convergedWindow = _currentTimeWindowNumber;

            _currentTimeWindowNumber++;
            _aitken.resetIteration();
            _timeWindowComplete = true;

            MINIMALCOUPLER_INFO("Window  has converged, Hence Advanced to window ", _currentTimeWindowNumber);

            writeWatchpointEntry(_currentTimeWindowNumber * _timeWindowSize, convergedWindow);

            _requiresWritingCheckPoint = true;
        } else {
            _requiresReadingCheckPoint = true;
            MINIMALCOUPLER_INFO("Window has not converged");
        }

        if (isCouplingOngoing()) {
            MINIMALCOUPLER_INFO("Waiting for Force from Fluid for window ", _currentTimeWindowNumber, "...");

            std::vector<double> forceData;
            Utils::recvVector(_remoteSocket, forceData);

            MINIMALCOUPLER_INFO(
                "Solid: Received ",
                forceData.size(),
                " force values for window ",
                _currentTimeWindowNumber
            );

            _providedMesh.addDataToMesh("Force", _currentTimeWindowNumber, std::move(forceData));

        } else {
            MINIMALCOUPLER_INFO("Solid: Coupling complete. No more data to receive.");

            int prevWindow = _currentTimeWindowNumber - 1;
            if (_providedMesh.checkIfTimeWindowExists("Force", prevWindow)) {
                auto lastForce = _providedMesh.getDataField("Force", prevWindow);
                _providedMesh.addDataToMesh("Force", _currentTimeWindowNumber, std::move(lastForce));
            }
        }
    }
}

} // namespace MinimalCoupler