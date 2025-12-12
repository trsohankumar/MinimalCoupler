cmake_minimum_required(VERSION 3.28)
project(minimalcoupler)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG v1.16.0
)
FetchContent_MakeAvailable(spdlog)

add_library(minimalcoupler
    minimalcoupler/Participant.hpp
    minimalcoupler/Participant.cpp
)

target_link_libraries(minimalcoupler
    PRIVATE spdlog::spdlog_header_only
)