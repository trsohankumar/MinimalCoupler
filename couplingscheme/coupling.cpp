#include "coupling.hpp"
#include <sys/socket.h>
#include <iostream>

namespace MinimalCoupler
{

    CouplingScheme::CouplingScheme()
        : _maxTime(0.0), _timeWindowSize(0.0), _currentTime(0.0)
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
            std::cout << "[FLUID] Sending " << forceSize << " force data to Solid" << std::endl;
            send(remoteSocket, fluidData.data(), forceSize * sizeof(double), 0);

            // Now it must get displacement data from the Solid participant
            size_t dispSize;
            recv(remoteSocket, &dispSize, sizeof(dispSize), 0);
            std::cout << "[FLUID] Receiving " << dispSize << " Displacement data from Solid" << std::endl;
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
            std::cout << "[SOLID] Receiving " << forceSize << " force data from Fluid" << std::endl;
            std::vector<double> forceData(forceSize);
            recv(remoteSocket, forceData.data(), forceSize * sizeof(double), MSG_WAITALL);

            mesh->addDataToMesh("Force", _currentTime, std::move(forceData));

            // send Displacement Data to Fluid
            const auto& solidData = mesh->getDataField("Displacement", _currentTime);
            size_t dispSize = solidData.size();
            send(remoteSocket, &dispSize, sizeof(dispSize), 0);
            std::cout << "[SOLID] Sending " << dispSize << " Displacement data to Fluid" << std::endl;
            send(remoteSocket, solidData.data(), dispSize * sizeof(double), 0);

        }

    }
}
