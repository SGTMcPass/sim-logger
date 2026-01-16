#include "logger/posix_time_source.hpp"

#include <chrono>

namespace sim_logger {

/**
 * @file posix_time_source.cpp
 * @brief Implementation of the stand-alone POSIX time source.
 *
 * @details
 * We use std::chrono::steady_clock for wall_time_ns() because it is monotonic:
 * it is guaranteed not to go backwards even if the system clock is adjusted.
 *
 * This is important for:
 *  - consistent ordering of log messages across threads, and
 *  - avoiding confusing timestamps during NTP corrections / time-of-day jumps.
 *
 * If real-world time-of-day is needed later, we can add it explicitly (e.g., system_clock).
 */

double PosixTimeSource::sim_time() noexcept {
  return 0.0;
}

double PosixTimeSource::mission_elapsed() noexcept {
  return 0.0;
}

int64_t PosixTimeSource::wall_time_ns() noexcept {
  using clock = std::chrono::steady_clock;
  const auto now = clock::now().time_since_epoch();
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
  return static_cast<int64_t>(ns);
}

} // namespace sim_logger
