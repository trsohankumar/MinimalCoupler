#pragma once
#include <vector>
#include <iomanip>

#include "logger.hpp"
#include "constants.hpp"
#include "Utils.hpp"

namespace MinimalCoupler
{
class Aitken
{
public:
    Aitken ();
    void checkConvergence(bool isConverged);
    bool isConverged() const;
    std::vector<double> getRelaxedData();
    void setRelaxedData(std::vector<double> data);
    void initialize(std::vector<double> initData);
    void resetIteration();
    void computeResiduals(std::vector<double> &newData);
    void updateRelaxation();
    void computeAitkenRelaxedOutput(std::vector<double> &outputData);
    double getResidualForLog() const;
    int getIterationNumber() const;

private:
    double _initialRelaxation;
    double _convergenceTolerance;
    bool _converged;
    int _maxIterations;
    double _omega;
    int _iterationNumber;
    std::vector<double> _data;
    std::vector<double> _currentResiduals;
    std::vector<double> _previousResiduals;
    double _residualForLog;
};
}