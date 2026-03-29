cmake_minimum_required(VERSION 3.10.2)
project(SolverdummyCPP LANGUAGES CXX DESCRIPTION "preCICE CPP solver dummy")

find_package(minimalCoupler REQUIRED CONFIG)

add_executable(solverdummy solverdummy.cpp)
set_target_properties(solverdummy PROPERTIES CXX_STANDARD 20)
target_link_libraries(solverdummy PRIVATE minimalCoupler::minimalCoupler)

