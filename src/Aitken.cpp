#include "Aitken.hpp"
#include "logger.hpp"

namespace MinimalCoupler
{

Aitken::Aitken ()
    : _initialRelaxation(Constants::INITIAL_RELAXATION), _convergenceTolerance(Constants::CONVERGENCE_TOLERANCE), _converged(false), _maxIterations(Constants::MAX_ITERATIONS), _omega(0.0), _iterationNumber(0)
{
}

void Aitken::checkConvergence(bool isConverged)
{

    if (isConverged)
    {
        _converged = true;
        MINIMALCOUPLER_INFO("Aitken converged after iteration:", _iterationNumber);
    }
    else if (_iterationNumber >= _maxIterations)
    {
        _converged = true;
        MINIMALCOUPLER_INFO("Convergence not acheived within maxIterations:  (", _maxIterations, "), Moving on to next window");
    }
    else
    {
        MINIMALCOUPLER_INFO("Aitken has not converged yet, iteration: ", _iterationNumber);
    }
}

bool Aitken::isConverged() const
{
    return _converged;
}

void Aitken::computeResiduals(std::vector<double> &newData)
{
    _currentResiduals.clear();
    
    // residuals is the difference between the new data and the old data
    for (size_t i = 0; i < newData.size(); i++)
    {
        _currentResiduals.push_back(newData.at(i) - _data.at(i));
    }
}

std::vector<double> Aitken::getRelaxedData()
{
    return _data;
}

void Aitken::updateRelaxation()
{
    if (_iterationNumber == 0)
    {
        if (_omega == 0.0)
        {
            // Very first window: no previous omega to preserve
            _omega = _initialRelaxation;
        }
        else
        {
            // preserve sign of previous omega, cap magnitude at initialRelaxation
            double sign = (_omega < 0.0) ? -1.0 : 1.0;
            _omega = sign * std::min(std::abs(_omega), _initialRelaxation);
        }
        MINIMALCOUPLER_INFO("Using initial relaxation for iteration 0, omega:", _omega);
    }
    else
    {
        // here we try to update omega
        // omega(i + 1) = -(omega(i))*((previosResidual)*(curentResidual - previousResidual)) / |((curentResidual - previousResidual)|^2)
        double numerator = 0.0;
        double denominator = 0.0;
        for (size_t i = 0; i < _currentResiduals.size(); i++)
        {
            double differance = _currentResiduals.at(i) - _previousResiduals.at(i);
            numerator += differance * _previousResiduals.at(i);
            denominator += differance * differance;
        }
        double prevOmega = _omega;
        _omega = -_omega * (numerator / denominator);
        MINIMALCOUPLER_INFO("Updated relaxation from ", prevOmega, " to ", _omega);
    }
}
void Aitken::computeAitkenRelaxedOutput(std::vector<double> &newData)
{
    MINIMALCOUPLER_INFO("Running Aitken for iteration number: ", _iterationNumber);

    _converged = false;
    computeResiduals(newData);

    // Compute L2 norm of new solver output for relative convergence measure
    double diffNorm = Utils::computeVectorNorm(Utils::vectorDifference(newData, _data));
    double newDataNorm = Utils::computeVectorNorm(newData);

    _residualForLog = (newDataNorm > 0.0) ? diffNorm / newDataNorm : 0.0;
    bool isConverged = diffNorm <= newDataNorm * _convergenceTolerance;

    checkConvergence(isConverged);

    if (_converged) 
    {
        _data = newData;
    }
    else 
    {
        updateRelaxation();

        // update formula: d(i+2) = d(i+1) + w(i+1)(residuals)
        for (size_t i = 0; i < _data.size(); i++)
        {
            _data[i] = _data[i] + _omega * _currentResiduals[i];
        }

        _previousResiduals = std::move(_currentResiduals);
        _iterationNumber++;
    }
}

void Aitken::resetIteration()
{
    _iterationNumber = 0;
}

void Aitken::setRelaxedData(std::vector<double> data)
{
    _data = data;
    MINIMALCOUPLER_INFO("Initialized Aitken _data with ", _data.size(), " values");
}

double Aitken::getResidualForLog() const
{
    return _residualForLog;
}

int Aitken::getIterationNumber() const
{
    return _iterationNumber;
}
}