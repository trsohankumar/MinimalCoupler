#pragma once

#include "impl/ParticipantImpl.hpp"

namespace MinimalCoupler
{
    class FluidParticipantImplementation: public ParticipantImplementation
    {

        FluidParticipantImplementation(
        std::string participantName,
        std::string configurationFileName,
        int         solverProcessIndex,
        int         solverProcessSize);

        FluidParticipantImplementation() = default;

    };
}