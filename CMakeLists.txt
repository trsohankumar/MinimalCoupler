cmake_minimum_required(VERSION 3.28)
project(minimalCoupler)

set(CMAKE_CXX_STANDARD 20)

add_library(minimalCoupler SHARED
    include/precice/precice.hpp
    include/precice/types.hpp
    include/precice/Participant.hpp
    src/precice/precice.cpp
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

set_target_properties(minimalCoupler PROPERTIES
    SOVERSION 3
)

target_include_directories(minimalCoupler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS minimalCoupler
    EXPORT minimalCouplerTargets
    LIBRARY DESTINATION lib
)

install (DIRECTORY include/precice/
    DESTINATION include/precice
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT minimalCouplerTargets
    NAMESPACE minimalCoupler::
    DESTINATION lib/cmake/minimalCoupler
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/minimalCouplerConfig.cmake"
      "include(\"\${CMAKE_CURRENT_LIST_DIR}/minimalCouplerTargets.cmake\")\n"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/minimalCouplerConfig.cmake"
    DESTINATION lib/cmake/minimalCoupler
)