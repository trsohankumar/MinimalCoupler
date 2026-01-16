#pragma once
#include "data/Mesh.hpp"
#include "precice/types.hpp"

namespace MinimalCoupler
{
    class CouplingScheme
    {
    public:
        CouplingScheme();
        double getMaxTime() const;
        double getTimeWinowSize() const;
        double getCurrentTime() const;
        void setMaxTime(double maxTime);
        void setTimeWindowSize(double timeWindowSize);

        void initialize(
            precice::string_view participantName,
            Mesh* mesh,
            int remoteSocket) const;
        
        void advance(
            precice::string_view participantName, 
            Mesh* mesh, 
            double computedTimeStepSize,
            int remoteSocket);

        bool isCouplingOnGoing() const;
        double getMaxTimeStepSize() const;
        bool isTimeWindowComplete() const;

    private:
        double _maxTime;
        double _timeWindowSize;
        int _currentTimeWindowNumber;
    };
}