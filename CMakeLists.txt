cmake_minimum_required (VERSION 3.2)
project (exradd)

include_directories (".")

add_executable (exradd exradd.cpp)

# Add C++11 features
target_compile_features(exradd PRIVATE cxx_range_for)
