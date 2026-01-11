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
        double getCurrentTimeWindow() const;
        void setCurrentTime(double time);
        void setMaxTime(double maxTime);
        void setTimeWindowSize(double timeWindowSize);
        void initialize(precice::string_view participantName, Mesh* mesh, int remoteSocket) const;
        bool isCouplingOnGoing() const;
        double getMaxTimeStepSize() const;

    private:
        double _maxTime;
        double _timeWindowSize;
        double _currentTime;
        int _currentTimeWindowNumber;
    };
}