add_library(logger_core
  src/level.cpp
  src/posix_time_source.cpp
  src/dummy_time_source.cpp
  src/test_sink.cpp
  src/logger.cpp
  src/logger_registry.cpp
  src/pattern_formatter.cpp
  src/global_time.cpp
  src/console_sink.cpp
  src/file_sink.cpp
  src/rotating_file_sink.cpp
)



add_library(sim_logger::core ALIAS logger_core)

target_include_directories(logger_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Keep core dependency-free in Sprint 1.
# (Later sprints may add fmt, json, etc.)
target_compile_features(logger_core PUBLIC cxx_std_17)

find_package(Threads REQUIRED)
target_link_libraries(logger_core PRIVATE Threads::Threads)

# Warnings (inherit top-level flags, but allow per-target additions later)
if(MSVC)
  target_compile_options(logger_core PRIVATE /W4)
else()
  target_compile_options(logger_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: define a project-wide export macro namespace later for shared libs.
# target_compile_definitions(logger_core PUBLIC LOGGER_CORE_EXPORTS=1)
