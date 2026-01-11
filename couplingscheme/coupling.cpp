#include "coupling.hpp"
#include <sys/socket.h>
#include <iostream>
#include "logger/logger.hpp"

namespace MinimalCoupler
{

    CouplingScheme::CouplingScheme()
        : _maxTime(0.0), _timeWindowSize(0.0), _currentTime(0.0), _currentTimeWindowNumber(1)
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
        return _currentTime;
    }

    void CouplingScheme::setCurrentTime(double time)
    {
        _currentTime = time;
    }

    void CouplingScheme::setMaxTime(double maxTime)
    {
        _maxTime = maxTime;
    }

    void CouplingScheme::setTimeWindowSize(double timeWindowSize)
    {
        _timeWindowSize = timeWindowSize;
    }

    void CouplingScheme::initialize(precice::string_view participantName, Mesh* mesh, int remoteSocket) const
    {
        if (participantName == "Fluid")
        {
            // send initial timestamp
            const auto& fluidData = mesh->getDataField("Force", _currentTime);
            send(remoteSocket, &_currentTime, sizeof(_currentTime), 0);

            // send data
            size_t forceSize = fluidData.size();
            send(remoteSocket, &forceSize, sizeof(forceSize), 0);
            MINIMALCOUPLER_INFO("Sending ", forceSize, " force data to Solid");
            send(remoteSocket, fluidData.data(), forceSize * sizeof(double), 0);

            // Now it must get displacement data from the Solid participant
            size_t dispSize;
            recv(remoteSocket, &dispSize, sizeof(dispSize), 0);
            MINIMALCOUPLER_INFO("Receiving ", dispSize, " Displacement data from Solid");
            std::vector<double> dispData(dispSize);
            recv(remoteSocket, dispData.data(), dispSize * sizeof(double), MSG_WAITALL);

            mesh->addDataToMesh("Displacement", _currentTime, std::move(dispData));
        }

        if (participantName == "Solid")
        {
            // recv initial timestamp
            double time;
            recv(remoteSocket, &time, sizeof(time), 0);

            // Now it must get force data from the Fluid participant
            size_t forceSize;
            recv(remoteSocket, &forceSize, sizeof(forceSize), 0);
            MINIMALCOUPLER_INFO("Receiving ", forceSize, " force data from Fluid");
            std::vector<double> forceData(forceSize);
            recv(remoteSocket, forceData.data(), forceSize * sizeof(double), MSG_WAITALL);

            mesh->addDataToMesh("Force", _currentTime, std::move(forceData));

            // send Displacement Data to Fluid
            const auto& solidData = mesh->getDataField("Displacement", _currentTime);
            size_t dispSize = solidData.size();
            send(remoteSocket, &dispSize, sizeof(dispSize), 0);
            MINIMALCOUPLER_INFO("Sending ", dispSize, " Displacement data to Fluid");
            send(remoteSocket, solidData.data(), dispSize * sizeof(double), 0);

        }

    }

    void CouplingScheme::advance(double computedTimeStepSize)
    {
        _currentTime += computedTimeStepSize;
        MINIMALCOUPLER_INFO("Time advanced by ", computedTimeStepSize, " to ", _currentTime);

        if (isTimeWindowComplete())
        {
            _currentTimeWindowNumber++;
            MINIMALCOUPLER_INFO("Time window ", _currentTimeWindowNumber - 1, " complete. Moving to window ", _currentTimeWindowNumber);
        }
    }

    bool CouplingScheme::isCouplingOnGoing() const
    {
        return !(_currentTime == _maxTime);
    }

    double CouplingScheme::getMaxTimeStepSize() const
    {
        return (_timeWindowSize * _currentTimeWindowNumber) - (_currentTime);
    }

    bool CouplingScheme::isTimeWindowComplete() const
    {
        return _currentTime >= (_timeWindowSize * _currentTimeWindowNumber);
    }
}
