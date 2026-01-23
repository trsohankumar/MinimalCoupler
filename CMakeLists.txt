cmake_minimum_required(VERSION 3.28)
project(minimalCoupler)

set(CMAKE_CXX_STANDARD 23)

add_library(minimalCoupler SHARED
    precice/precice.hpp
    precice/precice.cpp
    precice/types.hpp
    participant/Participant.hpp
    participant/Participant.cpp
    logger/logger.hpp
    logger/logger.cpp
    data/Mesh.hpp
    data/Mesh.cpp
    data/Point.hpp
    couplingscheme/coupling.hpp
    couplingscheme/coupling.cpp
    mapping/NearestNeighbor.hpp
    mapping/NearestNeighbor.cpp
    impl/ParticipantImpl.hpp
    impl/ParticipantImpl.cpp
)

set_target_properties(minimalCoupler PROPERTIES
    SOVERSION 3
)

target_include_directories(minimalCoupler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)