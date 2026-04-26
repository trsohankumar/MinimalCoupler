cmake_minimum_required(VERSION 3.16)
project(minimalCoupler)

set(CMAKE_CXX_STANDARD 20)

add_library(minimalCoupler SHARED
    include/precice/precice.hpp
    include/precice/Participant.hpp
    src/precice/Participant.cpp
    src/logger.hpp
    src/logger.cpp
    src/Mesh.hpp
    src/Mesh.cpp
    src/ParticipantImpl.hpp
    src/ParticipantImpl.cpp
    src/constants.hpp
    src/Aitken.hpp
    src/Aitken.cpp
    src/Utils.hpp
    src/Utils.cpp
)

target_link_libraries(minimalCoupler PRIVATE stdc++)

target_include_directories(minimalCoupler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(tests)