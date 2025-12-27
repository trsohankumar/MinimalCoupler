#include "ParticipantImpl.hpp"

namespace MinimalCoupler
{

    ParticipantImplementation::ParticipantImplementation(
        precice::string_view participantName,
        precice::string_view configurationFileName,
        int         solverProcessIndex,
        int         solverProcessSize)
        : _participantName(std::string(participantName)), _configFileName(std::string(configurationFileName)), _rank(solverProcessIndex), _size(solverProcessSize)

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