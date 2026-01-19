# tests/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)

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
    Catch2::Catch2WithMain
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
