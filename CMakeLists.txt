cmake_minimum_required(VERSION 3.20)

project(sim_logger LANGUAGES C CXX)

# -----------------------------
# Global options
# -----------------------------
option(SIM_LOGGER_USE_FETCHCONTENT "Allow CMake to fetch deps from the internet" OFF)

# Standard project-wide defaults
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest) # defines BUILD_TESTING

if (BUILD_TESTING)
  # -----------------------------
  # Dependencies (tests only)
  # -----------------------------
  # Resolve Catch2 only when tests are enabled. This keeps offline/non-test
  # builds working even when Catch2 is not vendored.

  set(SIM_LOGGER_CATCH2_EXTRAS_DIR "")

  if (EXISTS "${PROJECT_SOURCE_DIR}/third_party/Catch2/CMakeLists.txt")
    # Vendored Catch2
    set(_catch2_src "${PROJECT_SOURCE_DIR}/third_party/Catch2")
    set(_catch2_bin "${CMAKE_BINARY_DIR}/_deps/catch2-build")
    add_subdirectory("${_catch2_src}" "${_catch2_bin}" EXCLUDE_FROM_ALL)

    set(SIM_LOGGER_CATCH2_EXTRAS_DIR "${_catch2_src}/extras")

  else()
    # System Catch2 (preferred over network)
    find_package(Catch2 3 QUIET)

    if (Catch2_FOUND)
      # Usually include(Catch) will work without module-path tweaks.
      # If it doesn't on some platforms, vendoring is the recommended path.
    elseif (SIM_LOGGER_USE_FETCHCONTENT)
      include(FetchContent)
      FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4
      )
      FetchContent_MakeAvailable(Catch2)
      set(SIM_LOGGER_CATCH2_EXTRAS_DIR "${catch2_SOURCE_DIR}/extras")
    else()
      message(FATAL_ERROR
        "Catch2 not found.\n"
        "Provide ${PROJECT_SOURCE_DIR}/third_party/Catch2, install Catch2 v3, or configure with:\n"
        "  -DSIM_LOGGER_USE_FETCHCONTENT=ON\n"
      )
    endif()
  endif()

  if (NOT TARGET Catch2::Catch2WithMain)
    message(FATAL_ERROR "Catch2 resolved but target Catch2::Catch2WithMain does not exist.")
  endif()

  # Ensure catch_discover_tests is available even for vendored/fetched Catch2.
  if (SIM_LOGGER_CATCH2_EXTRAS_DIR AND EXISTS "${SIM_LOGGER_CATCH2_EXTRAS_DIR}/Catch.cmake")
    list(APPEND CMAKE_MODULE_PATH "${SIM_LOGGER_CATCH2_EXTRAS_DIR}")
  endif()
endif()

# -----------------------------
# Project targets
# -----------------------------
add_subdirectory(logger_core)

# Optional modules (keep whatever you already have; examples shown)
if (EXISTS "${PROJECT_SOURCE_DIR}/logger_c_api/CMakeLists.txt")
  add_subdirectory(logger_c_api)
endif()

if (EXISTS "${PROJECT_SOURCE_DIR}/logger_trick_adapter/CMakeLists.txt")
  add_subdirectory(logger_trick_adapter)
endif()

if (EXISTS "${PROJECT_SOURCE_DIR}/examples/CMakeLists.txt")
  add_subdirectory(examples)
endif()

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
