#pragma once

namespace MinimalCoupler::Constants {

const double MAX_TIME              = 5.0;
const double TIME_WINDOW_SIZE      = 0.01;
const double INITIAL_RELAXATION    = 0.5;
const int    MAX_ITERATIONS        = 50;
const int    MESH_DIMENSIONS       = 2;
const int    SERVER_PORT           = 5001;
const double CONVERGENCE_TOLERANCE = 5e-3;

const double WATCHPOINT_X          = 0.0;
const double WATCHPOINT_Y          = 1.0;
const double DOUBLE_EQUALITY_BOUND = 1e-10;
const bool   REQUIRES_INITIAL_DATA = false;
} // namespace MinimalCoupler::Constants