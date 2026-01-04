#include "coupling.hpp"

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

}
