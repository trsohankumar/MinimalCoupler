#include "coupling.hpp"
#include "logger/logger.hpp"
#include <iostream>
#include <sys/socket.h>

namespace MinimalCoupler
{

CouplingScheme::CouplingScheme() : _maxTime(0.0), _timeWindowSize(0.0), _currentTimeWindowNumber(0)
{
}

double CouplingScheme::getMaxTime() const
{
    return _maxTime;
}

double CouplingScheme::getTimeWinowSize() const
{
    return _timeWindowSize;
}

double CouplingScheme::getCurrentTime() const
{
    return _timeWindowSize * _currentTimeWindowNumber;
}

void CouplingScheme::setMaxTime(double maxTime)
{
    _maxTime = maxTime;
}

void CouplingScheme::setTimeWindowSize(double timeWindowSize)
{
    _timeWindowSize = timeWindowSize;
}

void CouplingScheme::initialize(precice::string_view participantName, Mesh *mesh, int remoteSocket) const
{
    double currentTime = getCurrentTime();

    if (participantName == "Fluid")
    {
        MINIMALCOUPLER_INFO("Retrieving Force data at time ", currentTime);
        const auto &fluidData = mesh->getDataField("Force", currentTime);

        // send data
        size_t forceSize = fluidData.size();
        MINIMALCOUPLER_INFO("Force data size is ", forceSize);
        send(remoteSocket, &forceSize, sizeof(forceSize), 0);
        MINIMALCOUPLER_INFO("Sending ", forceSize, " force data to Solid");
        send(remoteSocket, fluidData.data(), forceSize * sizeof(double), 0);

        // Now it must get displacement data from the Solid participant
        size_t dispSize;
        recv(remoteSocket, &dispSize, sizeof(dispSize), 0);
        MINIMALCOUPLER_INFO("Receiving ", dispSize, " Displacement data from Solid");
        std::vector<double> dispData(dispSize);
        recv(remoteSocket, dispData.data(), dispSize * sizeof(double), MSG_WAITALL);

        mesh->addDataToMesh("Displacement", currentTime, std::move(dispData));
    }

    if (participantName == "Solid")
    {
        // Get force data from the Fluid participant
        size_t forceSize;
        recv(remoteSocket, &forceSize, sizeof(forceSize), 0);
        MINIMALCOUPLER_INFO("Receiving ", forceSize, " force data from Fluid");
        std::vector<double> forceData(forceSize);
        recv(remoteSocket, forceData.data(), forceSize * sizeof(double), MSG_WAITALL);

        mesh->addDataToMesh("Force", currentTime, std::move(forceData));

        // send Displacement Data to Fluid
        const auto &solidData = mesh->getDataField("Displacement", currentTime);
        size_t dispSize = solidData.size();
        send(remoteSocket, &dispSize, sizeof(dispSize), 0);
        MINIMALCOUPLER_INFO("Sending ", dispSize, " Displacement data to Fluid");
        send(remoteSocket, solidData.data(), dispSize * sizeof(double), 0);
    }

    // The solid participant must wait until Fluid Finishes running its first time window
    if (participantName == "Solid")
    {
        size_t forceSize;
        recv(remoteSocket, &forceSize, sizeof(forceSize), 0);

        std::vector<double> forceData(forceSize);
        recv(remoteSocket, forceData.data(), forceSize * sizeof(double), MSG_WAITALL);

        // This data is stored at the end of first Time window
        double timeWindowEnd = _timeWindowSize * (_currentTimeWindowNumber + 1);

        MINIMALCOUPLER_INFO("Adding Force data to mesh for time:", timeWindowEnd);
        mesh->addDataToMesh("Force", timeWindowEnd, std::move(forceData));
    }
}

void CouplingScheme::advance(precice::string_view participantName, Mesh *mesh, double computedTimeStepSize,
                             int remoteSocket)
{

    // Check if this timestep completes the current window
    double nextTime = getCurrentTime() + computedTimeStepSize;
    double windowEnd = _timeWindowSize * (_currentTimeWindowNumber + 1);

    if (nextTime < windowEnd)
    {
        // Not at window end yet, don't advance coupling
        MINIMALCOUPLER_INFO("Time step of ", computedTimeStepSize, " does not complete window. Returning.");
        return;
    }

    // First both participants try to send any data that belongs to the current time window
    if (participantName == "Fluid")
    {
        // send data to Solid that it will use at next time window
        const auto &fluidData = mesh->getDataField("Force", getCurrentTime());

        size_t forceSize = fluidData.size();
        send(remoteSocket, &forceSize, sizeof(forceSize), 0);

        MINIMALCOUPLER_INFO("Sending ", forceSize, " force data to Solid at time: ", getCurrentTime());

        send(remoteSocket, fluidData.data(), forceSize * sizeof(double), 0);
    }

    if (participantName == "Solid")
    {

        const auto &solidData = mesh->getDataField("Displacement", getCurrentTime());

        size_t dispSize = solidData.size();

        send(remoteSocket, &dispSize, sizeof(dispSize), 0);

        MINIMALCOUPLER_INFO("Sending ", dispSize, " Displacement data to Fluid at time:", getCurrentTime());

        send(remoteSocket, solidData.data(), dispSize * sizeof(double), 0);
    }

    _currentTimeWindowNumber++;
    MINIMALCOUPLER_INFO("Time window: ", _currentTimeWindowNumber - 1,
                        " is complete. Moving to next time window. Start of next time window at: ", getCurrentTime());

    if (isCouplingOnGoing())
    {
        if (participantName == "Solid")
        {
            // As solid waits after initialize, every data it receives will be for the next time window

            size_t forceSize;
            recv(remoteSocket, &forceSize, sizeof(forceSize), 0);

            std::vector<double> forceData(forceSize);

            recv(remoteSocket, forceData.data(), forceSize * sizeof(double), MSG_WAITALL);

            double timeWindowEnd = _timeWindowSize * (_currentTimeWindowNumber + 1);

            MINIMALCOUPLER_INFO("Adding Force data to mesh for time:", timeWindowEnd);
            mesh->addDataToMesh("Force", timeWindowEnd, std::move(forceData));
        }
    }

    if (participantName == "Fluid")
    {

        // Fluid always receive data for the current time instance
        size_t dispSize;
        recv(remoteSocket, &dispSize, sizeof(dispSize), 0);

        std::vector<double> dispData(dispSize);
        recv(remoteSocket, dispData.data(), dispSize * sizeof(double), MSG_WAITALL);

        // This data is stored at the end of first Time window
        MINIMALCOUPLER_INFO("Adding Displacement data to mesh for time:", getCurrentTime());
        mesh->addDataToMesh("Displacement", getCurrentTime(), std::move(dispData));
    }
}

bool CouplingScheme::isCouplingOnGoing() const
{
    return getCurrentTime() < _maxTime;
}

double CouplingScheme::getMaxTimeStepSize() const
{
    // Returns time until the next window end
    return _timeWindowSize * (_currentTimeWindowNumber + 1) - getCurrentTime();
}

bool CouplingScheme::isTimeWindowComplete() const
{
    // always true as no substepping is involved
    return true;
}
} // namespace MinimalCoupler
