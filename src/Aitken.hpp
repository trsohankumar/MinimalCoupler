#pragma once
#include <iomanip>
#include <vector>

#include "Mesh.hpp"
#include "Utils.hpp"
#include "constants.hpp"
#include "logger.hpp"

namespace MinimalCoupler {
/**
 * @brief Implements the Aitken under-relaxation for accelerating convergence during implicit coupling
 * 
 * Used in each iteration to dynamically update omega using the
 * Aitken formula based on current and previous residuals.
 * Convergence is declared when the relative residual norm
 * drops below a tolerance, or the iteration limit is reached.
 */
class Aitken {
public:
    Aitken();
    void                updateConvergence(bool isConverged);
    bool                isConverged() const;
    std::vector<double> getRelaxedData() const;
    void                setRelaxedData(std::vector<double> data);
    void                initialize(std::vector<double> initData);
    void                resetIteration();
    void                computeResiduals(std::vector<double>& newData);
    void                updateRelaxation();
    void                computeAitkenRelaxedOutput(std::vector<double>& outputData);
    double              getResidualForLog() const;
    int                 getIterationNumber() const;
    bool                checkConvergence(std::vector<double>& newData);

private:
    double              _convergenceTolerance;
    bool                _converged;
    int                 _maxIterations;
    double              _omega;
    int                 _iterationNumber;
    std::vector<double> _data;
    std::vector<double> _currentResiduals;
    std::vector<double> _previousResiduals;
    double              _residualForLog;
};
} // namespace MinimalCoupler