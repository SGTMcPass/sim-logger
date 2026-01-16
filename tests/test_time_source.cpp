#include <catch2/catch_test_macros.hpp>

#include "logger/posix_time_source.hpp"
#include "logger/dummy_time_source.hpp"

#include <thread>

using namespace sim_logger;

TEST_CASE("PosixTimeSource returns monotonic wall_time_ns", "[time][posix]") {
  PosixTimeSource ts;

  const auto t1 = ts.wall_time_ns();

  // Small sleep to ensure time advances
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  const auto t2 = ts.wall_time_ns();

  REQUIRE(t2 > t1);
}

TEST_CASE("PosixTimeSource sim_time and mission_elapsed are zero", "[time][posix]") {
  PosixTimeSource ts;

  REQUIRE(ts.sim_time() == 0.0);
  REQUIRE(ts.mission_elapsed() == 0.0);
}

TEST_CASE("DummyTimeSource returns fixed initial values", "[time][dummy]") {
  DummyTimeSource ts(/*sim_time=*/12.5,
                     /*met=*/3.0,
                     /*wall_time_ns=*/1'000'000);

  REQUIRE(ts.sim_time() == 12.5);
  REQUIRE(ts.mission_elapsed() == 3.0);
  REQUIRE(ts.wall_time_ns() == 1'000'000);
}

TEST_CASE("DummyTimeSource advance updates all time values", "[time][dummy]") {
  DummyTimeSource ts(/*sim_time=*/0.0,
                     /*met=*/0.0,
                     /*wall_time_ns=*/0);

  ts.advance(/*sim_delta=*/1.5,
             /*met_delta=*/2.0,
             /*wall_delta_ns=*/500);

  REQUIRE(ts.sim_time() == 1.5);
  REQUIRE(ts.mission_elapsed() == 2.0);
  REQUIRE(ts.wall_time_ns() == 500);
}
