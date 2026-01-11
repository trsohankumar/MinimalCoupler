#include "ParticipantImpl.hpp"
#include "logger/logger.hpp"

namespace MinimalCoupler
{

    ParticipantImplementation::ParticipantImplementation(
        precice::string_view participantName,
        precice::string_view configurationFileName,
        int         solverProcessIndex,
        int         solverProcessSize)
        : _participantName(std::string(participantName)), _configFileName(std::string(configurationFileName)), couplingScheme(), _rank(solverProcessIndex), _size(solverProcessSize)

    {
        if (_participantName == "Solid")
        {
            _remoteParticipantName = "Fluid";
        }
        else
        {
            _remoteParticipantName = "Solid";
        }

        MINIMALCOUPLER_INFO("I am participant ", _participantName);
        MINIMALCOUPLER_INFO("My remote participant is ", _remoteParticipantName);
    }

    const std::string& ParticipantImplementation::getParticipantName() const
    {
        return _participantName;
    }

    const std::string& ParticipantImplementation::getRemoteParticipantName() const
    {
        return _remoteParticipantName;
    }

    const std::string& ParticipantImplementation::getConfigFileName() const
    {
        return _configFileName;
    }

    int ParticipantImplementation::getRank() const
    {
        return _rank;
    }

    int ParticipantImplementation::getSize() const
    {
        return _size;
    }

    CouplingScheme& ParticipantImplementation::getCouplingScheme() const
    {
        return couplingScheme;
    }
}