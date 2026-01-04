#pragma once

namespace MinimalCoupler
{
    class CouplingScheme
    {
    public:
        CouplingScheme();
        double getMaxTime() const;
        double getTimeWinowSize() const;
        double getCurrentTime() const;
        void setCurrentTime(double time);
        void setMaxTime(double maxTime);
        void setTimeWindowSize(double timeWindowSize);

    private:
        double _maxTime;
        double _timeWindowSize;
        double _currentTime;
    };
}