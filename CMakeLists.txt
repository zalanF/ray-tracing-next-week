cmake_minimum_required(VERSION 3.10.0)
project(RayTracer VERSION 0.1.0 LANGUAGES C CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
   src/main.cpp
   src/vec3.h
   src/color.h
   src/ray.h
   src/sphere.h
   src/hittable.h
   src/hittable_list.h
   src/rtutils.h
   src/interval.h
   src/camera.h
   src/material.h
   # src/Example.cpp
)

add_executable(RayTracer ${SOURCES})

