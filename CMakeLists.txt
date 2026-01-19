cmake_minimum_required(VERSION 3.10)
project(RType-All)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory for all binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Force all sub-projects to use the same output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(src/graphical-client)
add_subdirectory(src/server)

# Optional: Add a custom target to build everything
add_custom_target(all-projects
    DEPENDS rtype-client rtype-tcp-server rtype-udp-server
    COMMENT "Building all R-Type projects"
)