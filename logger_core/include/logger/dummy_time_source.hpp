#pragma once

#include "logger/time_source.hpp"

namespace sim_logger {

/**
 * @file dummy_time_source.hpp
 * @brief Deterministic time source for unit tests.
 *
 * @details
 * Logging infrastructure must be testable in isolation. Relying on real clocks
 * makes unit tests fragile and non-deterministic.
 *
 * DummyTimeSource provides a simple, fully deterministic implementation of
 * ITimeSource that returns user-controlled values. Tests can:
 *  - fix time at known values, or
 *  - advance time in a controlled manner.
 *
 * This class is intended strictly for testing and validation.
 */
class DummyTimeSource final : public ITimeSource {
 public:
  /**
   * @brief Construct a dummy time source with explicit initial values.
   *
   * @param sim_time Initial simulation time (seconds).
   * @param met Initial mission elapsed time (seconds).
   * @param wall_time_ns Initial monotonic timestamp (nanoseconds).
   */
  DummyTimeSource(double sim_time,
                  double met,
                  int64_t wall_time_ns) noexcept;

  ~DummyTimeSource() override = default;

  /// @return Current simulation time.
  double sim_time() noexcept override;

  /// @return Current mission elapsed time.
  double mission_elapsed() noexcept override;

  /// @return Current monotonic timestamp in nanoseconds.
  int64_t wall_time_ns() noexcept override;

  /**
   * @brief Advance all time values by the given deltas.
   *
   * @param sim_delta Seconds to add to simulation time.
   * @param met_delta Seconds to add to mission elapsed time.
   * @param wall_delta_ns Nanoseconds to add to wall time.
   */
  void advance(double sim_delta,
               double met_delta,
               int64_t wall_delta_ns) noexcept;

 private:
  double sim_time_;
  double met_;
  int64_t wall_time_ns_;
};

} // namespace sim_logger
