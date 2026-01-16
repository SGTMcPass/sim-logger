cmake_minimum_required(VERSION 3.20)

project(sim_logger
  VERSION 0.1.0
  LANGUAGES C CXX
)

# -----------------------------
# Options
# -----------------------------
option(LOGGER_ENABLE_TRICK "Build Trick adapter components (requires Trick headers/libs)" OFF)
option(LOGGER_BUILD_TESTS  "Build unit tests" ON)
option(LOGGER_BUILD_EXAMPLES "Build examples" ON)

# -----------------------------
# Global build settings
# -----------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Helpful defaults for debug builds
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Enable warnings (portable-ish baseline)
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -----------------------------
# Subprojects
# -----------------------------
add_subdirectory(logger_core)
#add_subdirectory(logger_c_api)

if(LOGGER_ENABLE_TRICK)
  add_subdirectory(logger_trick_adapter)
endif()

if(LOGGER_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(LOGGER_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

