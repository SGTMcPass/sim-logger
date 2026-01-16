include(FetchContent)

# Catch2 v3 (header-only usage with a small main)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.4
)

# NOTE: Catch2 requires C++14 or later, set_property(...) ensures it uses C++17
#       without muddling versions in other places.
FetchContent_MakeAvailable(Catch2)

set_property(TARGET Catch2 Catch2WithMain PROPERTY CXX_STANDARD 17)
set_property(TARGET Catch2 Catch2WithMain PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET Catch2 Catch2WithMain PROPERTY CXX_EXTENSIONS NO)

add_executable(sim_logger_tests
  test_level.cpp
  test_time_source.cpp
  test_log_record.cpp
  test_sprint2_logger_registry.cpp
)

target_link_libraries(sim_logger_tests
  PRIVATE
    sim_logger::core
    Catch2::Catch2WithMain
)

target_compile_features(sim_logger_tests PRIVATE cxx_std_17)

if(MSVC)
  target_compile_options(sim_logger_tests PRIVATE /W4)
else()
  target_compile_options(sim_logger_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
include(Catch)
catch_discover_tests(sim_logger_tests)
