cmake_minimum_required(VERSION 3.28)
project(minimalCoupler)

set(CMAKE_CXX_STANDARD 20)

add_library(minimalCoupler SHARED
    src/precice/precice.hpp
    src/precice/precice.cpp
    src/precice/types.hpp
    src/Participant.hpp
    src/Participant.cpp
    src/logger.hpp
    src/logger.cpp
    src/Mesh.hpp
    src/Mesh.cpp
    src/Point.hpp
    src/ParticipantImpl.hpp
    src/ParticipantImpl.cpp
    src/constants.hpp
    src/Aitken.hpp
    src/Aitken.cpp
    src/Utils.hpp
    src/Utils.cpp
)

set_target_properties(minimalCoupler PROPERTIES
    SOVERSION 3
)

target_include_directories(minimalCoupler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)