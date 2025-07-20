cmake_minimum_required(VERSION 3.10)
project(AnomDetect CXX)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)
add_executable(anom_detect
  src/metrics_impl.cpp
  src/main.cpp
)