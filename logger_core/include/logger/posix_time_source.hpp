#pragma once

#include "logger/time_source.hpp"

namespace sim_logger {

/**
 * @file posix_time_source.hpp
 * @brief Stand-alone POSIX implementation of ITimeSource.
 *
 * @details
 * This implementation is used when the logger is running:
 *  - outside of Trick, or
 *  - in unit tests and tools that do not link against Trick.
 *
 * Design intent:
 *  - Provide a reasonable default time source with **zero external dependencies**.
 *  - Emphasize correctness, monotonicity, and portability over absolute wall-clock meaning.
 *
 * Behavior:
 *  - sim_time(): returns 0.0 (no simulation executive available)
 *  - mission_elapsed(): returns 0.0
 *  - wall_time_ns(): returns a **monotonically increasing** timestamp suitable for ordering
 *
 * In Trick-based runs, this class is replaced by TrickTimeSource via the optional adapter.
 */
class PosixTimeSource final : public ITimeSource {
 public:
  PosixTimeSource() = default;
  ~PosixTimeSource() override = default;

  /**
   * @brief Return simulation time.
   *
   * @return Always 0.0 in the stand-alone POSIX implementation.
   *
   * @note
   * This avoids inventing semantics that may conflict with Trick.
   * Non-Trick users may subclass ITimeSource if they want custom behavior.
   */
  double sim_time() noexcept override;

  /**
   * @brief Return mission elapsed time (MET).
   *
   * @return Always 0.0 in the stand-alone POSIX implementation.
   */
  double mission_elapsed() noexcept override;

  /**
   * @brief Return a monotonic timestamp in nanoseconds.
   *
   * @details
   * Uses a monotonic clock so that:
   *  - values never go backwards, and
   *  - ordering across threads is stable.
   *
   * @return Monotonic time in nanoseconds.
   */
  int64_t wall_time_ns() noexcept override;
};

} // namespace sim_logger
