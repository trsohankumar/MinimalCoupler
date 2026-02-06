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
    int getCurrentWindowNumber() const;
    void setMaxTime(double maxTime);
    void setTimeWindowSize(double timeWindowSize);
    void setInitialRelaxation(double initialRelaxation);
    void setMaxIterations(int maxIterations);
    void initialize(precice::string_view participantName, Mesh *mesh, int remoteSocket);

    void advance(precice::string_view participantName, Mesh *mesh, double computedTimeStepSize, int remoteSocket);
    bool requiresWritingCheckpoing();
    bool requiresReadingCheckpoing();

    bool isCouplingOnGoing() const;
    double getMaxTimeStepSize() const;
    bool isTimeWindowComplete() const;
    void checkConvergence();
    double getL2NormOfResiduals() const;

  private:
    void setPreviousData(std::vector<double> previousData);
    void computeResiduals(std::vector<double> &newData);
    void setPreviousResiduals(std::vector<double> &oldResiduals);
    void updateRelaxation();
    void computeAitkenRelaxedOutput(std::vector<double> &outputData);
    double computeScaledResidualNorm() const;

    void enableRequiresWritingCheckpoint();
    void enableRequiresReadingCheckpoint();
    void disableRequiresWritingCheckpoint();
    void disableRequiresReadingCheckpoint();

    double _maxTime;
    double _timeWindowSize;
    double _initialRelaxation;
    double _omega;
    double _convergenceTolerance;
    int _iterationNumber;
    int _maxIterations;
    int _currentTimeWindowNumber;
    bool _converged;
    bool _requiresWritingCheckPoint;
    bool _requiresReadingCheckPoint;
    std::vector<double> _data;
    std::vector<double> _currentResiduals;
    std::vector<double> _previousResiduals;
};
} // namespace MinimalCoupler