#include "coupling.hpp"
#include "logger.hpp"
#include "socketutils.hpp"
#include "constants.hpp"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <math.h>

namespace MinimalCoupler
{

using utils::sendAll;
using utils::recvAll;

CouplingScheme::CouplingScheme()
    : _maxTime(0.0), _timeWindowSize(0.0), _initialRelaxation(0.0), _omega(0.0), _convergenceTolerance(Constants::CONVERGENCE_TOLERANCE),
      _iterationNumber(0), _maxIterations(50), _currentTimeWindowNumber(0), _converged(false),
      _requiresWritingCheckPoint(false), _requiresReadingCheckPoint(false),
      _data(0), _currentResiduals(0), _previousResiduals(0)
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
    return _timeWindowSize * _currentTimeWindowNumber;
}

int CouplingScheme::getCurrentWindowNumber() const
{
    return _currentTimeWindowNumber;
}

void CouplingScheme::setMaxTime(double maxTime)
{
    _maxTime = maxTime;
}

void CouplingScheme::setTimeWindowSize(double timeWindowSize)
{
    _timeWindowSize = timeWindowSize;
}

void CouplingScheme::setInitialRelaxation(double initialRelaxation)
{
    _initialRelaxation = initialRelaxation;
}

void CouplingScheme::setMaxIterations(int maxIterations)
{
    _maxIterations = maxIterations;
}

void CouplingScheme::setPreviousData(std::vector<double> previousData)
{
    _data = std::move(previousData);
}

void CouplingScheme::computeResiduals(std::vector<double> &newData)
{
    _currentResiduals.clear();
    
    // residuals is the difference between the new data and the old data
    for (size_t i = 0; i < newData.size(); i++)
    {
        _currentResiduals.push_back(newData.at(i) - _data.at(i));
    }
}

void CouplingScheme::updateRelaxation()
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

double CouplingScheme::computeVectorNorm(const std::vector<double>& data) const
{
    double squaredSum = 0.0;
    for (auto it: data)
    {
        squaredSum += it * it;
    }

    return std::sqrt(squaredSum);
}

std::vector<double> CouplingScheme::vectorDifference(std::vector<double>& first, std::vector<double>& second)
{
    std::vector<double> diff(first.size());
    for(size_t i = 0; i < first.size(); i++)
        diff[i] = first[i] - second[i];

    return diff;
}

void CouplingScheme::computeAitkenRelaxedOutput(std::vector<double> &newData)
{
    MINIMALCOUPLER_INFO("Running Aitken relaxation for window number:", _currentTimeWindowNumber, ", and iteration number: ", _iterationNumber);

    _converged = false;
    computeResiduals(newData);

    // Compute L2 norm of new solver output for relative convergence measure
    double diffNorm = computeVectorNorm(vectorDifference(newData, _data));
    double newDataNorm = computeVectorNorm(newData);

    bool isConverged = diffNorm <= newDataNorm * _convergenceTolerance;

    checkConvergence(isConverged);
    // Write values to file for debugging
    if (_convergenceLog.is_open())
    {
        _convergenceLog << _currentTimeWindowNumber << "\t"
                        << _iterationNumber << "\t"
                        << std::scientific << std::setprecision(6) << diffNorm / newDataNorm << "\t"  
                        << (_converged ? 1 : 0) << std::endl;
    }

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

void CouplingScheme::checkConvergence(bool isConverged)
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

void CouplingScheme::openConvergenceLog()
{
    _convergenceLog.open("convergence.log", std::ios::out | std::ios::trunc);
    _convergenceLog << "TimeWindow\tIteration\tResRel\tConverged" << std::endl;
}

void CouplingScheme::initialize(precice::string_view participantName, Mesh *mesh, int remoteSocket)
{
    MINIMALCOUPLER_INFO("Current window = ", _currentTimeWindowNumber);

    if (participantName == "Fluid")
    {
        MINIMALCOUPLER_INFO("Retrieving Force data at window ", _currentTimeWindowNumber);
        const auto &fluidData = mesh->getDataField("Force", _currentTimeWindowNumber);

        size_t forceSize = fluidData.size();
        sendAll(remoteSocket, &forceSize, sizeof(forceSize));
        MINIMALCOUPLER_INFO("Sending ", forceSize, " initial force values to Solid");
        sendAll(remoteSocket, fluidData.data(), forceSize * sizeof(double));

        size_t dispSize;
        recvAll(remoteSocket, &dispSize, sizeof(dispSize));
        std::vector<double> dispData(dispSize);
        recvAll(remoteSocket, dispData.data(), dispSize * sizeof(double));
        MINIMALCOUPLER_INFO("Received ", dispSize, " initial displacement values from Solid");

        mesh->addDataToMesh("Displacement", _currentTimeWindowNumber, std::move(dispData));
        MINIMALCOUPLER_INFO("Initialize complete");
    }

    if (participantName == "Solid")
    {
        size_t forceSize;
        recvAll(remoteSocket, &forceSize, sizeof(forceSize));
        std::vector<double> forceData(forceSize);
        recvAll(remoteSocket, forceData.data(), forceSize * sizeof(double));
        MINIMALCOUPLER_INFO("Received ", forceSize, " initial force values from Fluid");

        mesh->addDataToMesh("Force", _currentTimeWindowNumber, std::move(forceData));

        const auto &solidData = mesh->getDataField("Displacement", _currentTimeWindowNumber);
        size_t dispSize = solidData.size();
        sendAll(remoteSocket, &dispSize, sizeof(dispSize));
        MINIMALCOUPLER_INFO("Sending ", dispSize, " initial displacement values to Fluid");
        sendAll(remoteSocket, solidData.data(), dispSize * sizeof(double));

        // Initialize _data for Aitken relaxation with current displacement as first guess
        _data = std::vector<double>(solidData.begin(), solidData.end());
        MINIMALCOUPLER_INFO("Initialized Aitken _data with ", _data.size(), " values");

        openConvergenceLog();
    }

    // Solid blocks waiting for first Force from Fluid's advance()
    if (participantName == "Solid")
    {
        MINIMALCOUPLER_INFO("Blocking, waiting for Force from Fluid's first advance()...");

        size_t forceSize;
        recvAll(remoteSocket, &forceSize, sizeof(forceSize));
        std::vector<double> forceData(forceSize);
        recvAll(remoteSocket, forceData.data(), forceSize * sizeof(double));

        // This Force is for solving window 0, so store at current window
        MINIMALCOUPLER_INFO("Received ", forceSize, " force values for window ", _currentTimeWindowNumber);
        mesh->addDataToMesh("Force", _currentTimeWindowNumber, std::move(forceData));
        MINIMALCOUPLER_INFO("Initialize complete, ready to run solver");
    }

    enableRequiresWritingCheckpoint();
}

void CouplingScheme::enableRequiresWritingCheckpoint()
{
    _requiresWritingCheckPoint = true;
}

void CouplingScheme::enableRequiresReadingCheckpoint()
{
    _requiresReadingCheckPoint = true;
}

void CouplingScheme::disableRequiresWritingCheckpoint()
{

    _requiresWritingCheckPoint = false;
}

void CouplingScheme::disableRequiresReadingCheckpoint()
{

    _requiresReadingCheckPoint = false;
}

bool CouplingScheme::requiresWritingCheckpoing()
{
    bool res = _requiresWritingCheckPoint;
    if (_requiresWritingCheckPoint)
    {
        disableRequiresWritingCheckpoint();
    }
    return res;
}

bool CouplingScheme::requiresReadingCheckpoing()
{
    bool res = _requiresReadingCheckPoint;
    if (_requiresReadingCheckPoint)
    {
        disableRequiresReadingCheckpoint();
    }
    return res;
}
void CouplingScheme::advance(precice::string_view participantName, Mesh *mesh, double computedTimeStepSize,
                             int remoteSocket)
{
    if (participantName == "Fluid")
    {
        MINIMALCOUPLER_INFO("Advance with window number", _currentTimeWindowNumber);

        // Validate Force data exists at current window
        if (!mesh->checkIfTimeWindowExists("Force", _currentTimeWindowNumber))
        {
            MINIMALCOUPLER_INFO("ERROR: Force data does not exist at window ", _currentTimeWindowNumber, "!");
            auto windows = mesh->getAvailableTimeWindows("Force");
            MINIMALCOUPLER_INFO("Available Force windows: ");
            for (int w : windows)
            {
                MINIMALCOUPLER_INFO("  - ", w);
            }
            return;
        }

        // Send Force for current iteration
        const auto &fluidData = mesh->getDataField("Force", _currentTimeWindowNumber);
        size_t forceSize = fluidData.size();
        sendAll(remoteSocket, &forceSize, sizeof(forceSize));
        MINIMALCOUPLER_INFO("Fluid: Sending ", forceSize, " force values to Solid");
        sendAll(remoteSocket, fluidData.data(), forceSize * sizeof(double));

        // Block waiting for Displacement + convergence from Solid
        MINIMALCOUPLER_INFO("Waiting for Displacement data and  convergence data from Solid...");
        size_t dispSize;
        recvAll(remoteSocket, &dispSize, sizeof(dispSize));
        std::vector<double> dispData(dispSize);
        recvAll(remoteSocket, dispData.data(), dispSize * sizeof(double));
        recvAll(remoteSocket, &_converged, sizeof(_converged));

        // Store received displacement at current window
        MINIMALCOUPLER_INFO("Received displacement values numbered:", dispSize, " and converged: ", _converged);
        mesh->addDataToMesh("Displacement", _currentTimeWindowNumber, std::move(dispData));

        if (_converged)
        {
            _currentTimeWindowNumber++;
            MINIMALCOUPLER_INFO("Window converged, Hence Advanced to window ", _currentTimeWindowNumber);
            enableRequiresWritingCheckpoint();
        }
        else
        {
            enableRequiresReadingCheckpoint();
            MINIMALCOUPLER_INFO(" Window has not converged");
        }
    }

    if (participantName == "Solid")
    {
        MINIMALCOUPLER_INFO("Advance with window number", _currentTimeWindowNumber);

        // Validate Displacement data exists at current window
        if (!mesh->checkIfTimeWindowExists("Displacement", _currentTimeWindowNumber))
        {
            MINIMALCOUPLER_INFO("ERROR: Displacement data does not exist at window ", _currentTimeWindowNumber, "!");
            auto windows = mesh->getAvailableTimeWindows("Displacement");
            MINIMALCOUPLER_INFO("Available Displacement windows: ");
            for (int w : windows)
            {
                MINIMALCOUPLER_INFO("  - ", w);
            }
            return;
        }

        // Solid already has Force (received in initialize or previous advance)
        // Solver has run, now get raw Displacement output
        const auto &rawDispData = mesh->getDataField("Displacement", _currentTimeWindowNumber);
        MINIMALCOUPLER_INFO("Solid: Got raw displacement (", rawDispData.size(), " values) from solver");

        // Apply Aitken relaxation and check convergence
        std::vector<double> rawDispVec(rawDispData.begin(), rawDispData.end());
        computeAitkenRelaxedOutput(rawDispVec);

        // Send RELAXED displacement (_data), not raw
        size_t dispSize = _data.size();
        sendAll(remoteSocket, &dispSize, sizeof(dispSize));
        MINIMALCOUPLER_INFO("Sending ", dispSize, " relaxed displacement values to Fluid");
        sendAll(remoteSocket, _data.data(), dispSize * sizeof(double));

        MINIMALCOUPLER_INFO("Sending convergence status: ", _converged);
        sendAll(remoteSocket, &_converged, sizeof(_converged));

        if (_converged)
        {
            _currentTimeWindowNumber++;
            _iterationNumber = 0;
            MINIMALCOUPLER_INFO("Window  has converged, Hence Advanced to window ", _currentTimeWindowNumber);
            enableRequiresWritingCheckpoint();
        }
        else
        {
            enableRequiresReadingCheckpoint();
            MINIMALCOUPLER_INFO("Window has notconverged");
        }

        if (isCouplingOnGoing())
        {
            MINIMALCOUPLER_INFO("Waiting for Force from Fluid for window ", _currentTimeWindowNumber, "...");

            size_t forceSize;
            recvAll(remoteSocket, &forceSize, sizeof(forceSize));
            std::vector<double> forceData(forceSize);
            recvAll(remoteSocket, forceData.data(), forceSize * sizeof(double));

            MINIMALCOUPLER_INFO("Solid: Received ", forceSize, " force values for window ", _currentTimeWindowNumber);
            mesh->addDataToMesh("Force", _currentTimeWindowNumber, std::move(forceData));
        }
        else
        {
            MINIMALCOUPLER_INFO("Solid: Coupling complete. No more data to receive.");
        
            int prevWindow = _currentTimeWindowNumber - 1;
            if (mesh->checkIfTimeWindowExists("Force", prevWindow))
            {
                auto lastForce = mesh->getDataField("Force", prevWindow);
                mesh->addDataToMesh("Force", _currentTimeWindowNumber, std::move(lastForce));
            }
        }
    }
}

bool CouplingScheme::isCouplingOnGoing() const
{
    return getCurrentTime() < _maxTime;
}

double CouplingScheme::getMaxTimeStepSize() const
{
    // Returns time until the next window end
    return _timeWindowSize * (_currentTimeWindowNumber + 1) - getCurrentTime();
}

bool CouplingScheme::isTimeWindowComplete() const
{
    return _converged;
}
} // namespace MinimalCoupler
