#include "FluidParticipantImpl.hpp"

namespace MinimalCoupler
{

    FluidParticipantImplementation::FluidParticipantImplementation(std::string participantName,
    std::string configurationFileName,int solverProcessIndex, int solverProcessSize)
        : ParticipantImplementation(participantName, configurationFileName, solverProcessIndex, solverProcessSize)
    {

    }

}