#include "ParticipantImpl.hpp"

namespace MinimalCoupler
{

    ParticipantImplementation::ParticipantImplementation(
        std::string participantName,
        std::string configurationFileName,
        int         solverProcessIndex,
        int         solverProcessSize)
        : _participantName(std::move(participantName)), _configFileName(std::move(configurationFileName)), _rank(solverProcessIndex), _size(solverProcessSize)

    {
        if (_participantName == "Solid")
        {
            _remoteParticipantName = "Fluid";
        }
        else 
        {
            _remoteParticipantName = "Solid";
        }
    }
}