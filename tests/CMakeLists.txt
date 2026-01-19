# tests/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)

# Allow top-level to define this; otherwise default OFF for reproducible/offline builds.
if (NOT DEFINED SIM_LOGGER_USE_FETCHCONTENT)
  option(SIM_LOGGER_USE_FETCHCONTENT "Allow CMake to fetch deps from the internet" OFF)
endif()

# -----------------------------
# Catch2 dependency resolution
# -----------------------------
set(SIM_LOGGER_CATCH2_TARGET "")
set(SIM_LOGGER_CATCH2_EXTRAS_DIR "")

# 1) Vendored Catch2 (recommended): third_party/Catch2
if (EXISTS "${PROJECT_SOURCE_DIR}/third_party/Catch2/CMakeLists.txt")
  add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/Catch2" EXCLUDE_FROM_ALL)
  set(SIM_LOGGER_CATCH2_TARGET Catch2::Catch2WithMain)
  set(SIM_LOGGER_CATCH2_EXTRAS_DIR "${PROJECT_SOURCE_DIR}/third_party/Catch2/extras")

# 2) System Catch2 (e.g., apt/homebrew/vcpkg)
else()
  find_package(Catch2 3 QUIET)

  if (Catch2_FOUND)
    set(SIM_LOGGER_CATCH2_TARGET Catch2::Catch2WithMain)

    # Catch2 installs Catch.cmake to a known location; use the package-provided module path.
    # CMake usually finds it automatically after find_package(Catch2), but keep this robust.
    if (DEFINED Catch2_DIR)
      # Catch2_DIR points at the package config dir; extras are usually alongside modules.
      # If include(Catch) fails on some setups, prefer vendoring or add a toolchain-specific fix.
    endif()

  # 3) FetchContent (opt-in only)
  elseif (SIM_LOGGER_USE_FETCHCONTENT)
    include(FetchContent)

    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.5.4
    )

    FetchContent_MakeAvailable(Catch2)

    set(SIM_LOGGER_CATCH2_TARGET Catch2::Catch2WithMain)
    set(SIM_LOGGER_CATCH2_EXTRAS_DIR "${catch2_SOURCE_DIR}/extras")

  else()
    message(FATAL_ERROR
      "Catch2 not found.\n"
      "Provide ${PROJECT_SOURCE_DIR}/third_party/Catch2, install Catch2 v3, or configure with:\n"
      "  -DSIM_LOGGER_USE_FETCHCONTENT=ON\n"
    )
  endif()
endif()

if (NOT TARGET ${SIM_LOGGER_CATCH2_TARGET})
  message(FATAL_ERROR "Resolved Catch2 target '${SIM_LOGGER_CATCH2_TARGET}' does not exist.")
endif()

# Ensure Catch2 uses C++17 (matches project)
set_property(TARGET Catch2 Catch2WithMain PROPERTY CXX_STANDARD 17)
set_property(TARGET Catch2 Catch2WithMain PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET Catch2 Catch2WithMain PROPERTY CXX_EXTENSIONS NO)

# Make catch_discover_tests() available when using vendored or fetched Catch2.
# (System installs usually make include(Catch) work without this, but it doesn't hurt.)
if (SIM_LOGGER_CATCH2_EXTRAS_DIR AND EXISTS "${SIM_LOGGER_CATCH2_EXTRAS_DIR}/Catch.cmake")
  list(APPEND CMAKE_MODULE_PATH "${SIM_LOGGER_CATCH2_EXTRAS_DIR}")
endif()

# -----------------------------
# Test executable
# -----------------------------
add_executable(sim_logger_tests
  test_level.cpp
  test_time_source.cpp
  test_log_record.cpp
  test_sprint2_logger_registry.cpp
  test_pattern_formatter.cpp
)

target_link_libraries(sim_logger_tests
  PRIVATE
    sim_logger::core
    ${SIM_LOGGER_CATCH2_TARGET}
)

target_compile_features(sim_logger_tests PRIVATE cxx_std_17)

if (MSVC)
  target_compile_options(sim_logger_tests PRIVATE /W4)
else()
  target_compile_options(sim_logger_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
include(Catch)
catch_discover_tests(sim_logger_tests)
