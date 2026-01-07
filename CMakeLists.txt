cmake_minimum_required(VERSION 3.28)
project(precice)

set(CMAKE_CXX_STANDARD 23)

add_library(precice SHARED
    precice/precice.hpp
    precice/precice.cpp
    precice/types.hpp
    participant/Participant.hpp
    participant/Participant.cpp
    data/Mesh.hpp
    data/Mesh.cpp
    data/Point.hpp
    couplingscheme/coupling.hpp
    couplingscheme/coupling.cpp
    mapping/NearestNeighbor.hpp
    mapping/NearestNeighbor.cpp
    impl/ParticipantImpl.hpp
    impl/ParticipantImpl.cpp
    fluid/FluidParticipantImpl.hpp
    fluid/FluidParticipantImpl.cpp
    solid/SolidParticipantImpl.hpp
    solid/SolidParticipantImpl.cpp
)

set_target_properties(precice PROPERTIES
    SOVERSION 3
)

target_include_directories(precice
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)