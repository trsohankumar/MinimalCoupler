#pragma once

#include "impl/ParticipantImpl.hpp"

namespace MinimalCoupler
{
    class SolidParticipantImplementation: public ParticipantImplementation
    {

        SolidParticipantImplementation(
        std::string participantName,
        std::string configurationFileName,
        int         solverProcessIndex,
        int         solverProcessSize);

        SolidParticipantImplementation() = default;

    };
}