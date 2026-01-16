#pragma once

#include <cstdint>

namespace sim_logger {

/**
 * @file time_source.hpp
 * @brief Defines the abstraction for retrieving time values used in log records.
 *
 * @details
 * The logger must stamp each message with multiple notions of time:
 *  - Simulation time (from the simulation executive / scheduler)
 *  - Mission Elapsed Time (MET)
 *  - A high-resolution host timestamp for ordering and correlation
 *
 * Trick provides simulation time and MET in Trick-based runs, but the logger must also work
 * without Trick (unit tests and stand-alone usage). To avoid making the core logger depend
 * on Trick headers, we define an interface that any environment can implement.
 *
 * In Sprint 1 we provide:
 *  - PosixTimeSource: stand-alone time source for non-Trick usage
 *  - DummyTimeSource: deterministic time source for unit tests
 *
 * Later sprints will add:
 *  - TrickTimeSource: integrates with Trick's scheduler/time base (optional build component)
 */

/**
 * @brief Interface for retrieving time values used in logging.
 *
 * @note
 * This interface is intentionally small and noexcept: logging must never destabilize the simulation.
 *
 * @warning
 * Be explicit about what your "wall time" represents. In many systems:
 *  - "wall clock" means real-world time-of-day, and
 *  - "monotonic" means a steadily increasing timer suitable for ordering.
 *
 * For logging, monotonic time is usually preferable for ordering across threads.
 * We therefore define wall_time_ns() as a monotonic timestamp in nanoseconds
 * in the stand-alone implementation. (If true time-of-day is needed later, it can be added
 * as an additional field or provided by a formatter.)
 */
class ITimeSource {
 public:
  virtual ~ITimeSource() = default;

  /**
   * @brief Return current simulation time (seconds).
   *
   * @details
   * In Trick-based runs this should be sourced from the Trick scheduler time base.
   * In stand-alone runs it may be 0.0 or an application-defined value.
   */
  virtual double sim_time() noexcept = 0;

  /**
   * @brief Return current mission elapsed time (MET) (seconds).
   *
   * @details
   * MET is often defined as seconds since scenario start (T0). In Trick-based runs,
   * this should be aligned with the simulation's MET convention.
   */
  virtual double mission_elapsed() noexcept = 0;

  /**
   * @brief Return a monotonically increasing host timestamp (nanoseconds).
   *
   * @details
   * This timestamp is used for stable ordering/correlation of log messages across threads.
   * It is not intended to represent real-world time-of-day.
   */
  virtual int64_t wall_time_ns() noexcept = 0;
};

} // namespace sim_logger
