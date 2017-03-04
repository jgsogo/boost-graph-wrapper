project(boost-graph-wrapper)
cmake_minimum_required(VERSION 2.8.12)

include (GenerateExportHeader)

find_package (Boost COMPONENTS graph REQUIRED)

file(GLOB sources
    "*.h"
    "*.cpp"
    "*.hpp"
)

add_library(boost-graph-wrapper ${sources})
generate_export_header(boost-graph-wrapper)
set_target_properties(boost-graph-wrapper PROPERTIES LINKER_LANGUAGE CXX)

