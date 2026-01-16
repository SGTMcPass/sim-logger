#include "logger/dummy_time_source.hpp"

namespace sim_logger {

/**
 * @file dummy_time_source.cpp
 * @brief Implementation of the deterministic test time source.
 */

DummyTimeSource::DummyTimeSource(double sim_time,
                                 double met,
                                 int64_t wall_time_ns) noexcept
  : sim_time_(sim_time),
    met_(met),
    wall_time_ns_(wall_time_ns) {}

double DummyTimeSource::sim_time() noexcept {
  return sim_time_;
}

double DummyTimeSource::mission_elapsed() noexcept {
  return met_;
}

int64_t DummyTimeSource::wall_time_ns() noexcept {
  return wall_time_ns_;
}

void DummyTimeSource::advance(double sim_delta,
                              double met_delta,
                              int64_t wall_delta_ns) noexcept {
  sim_time_ += sim_delta;
  met_ += met_delta;
  wall_time_ns_ += wall_delta_ns;
}

} // namespace sim_logger
