#include <catch2/catch_test_macros.hpp>

#include "logger/logger_registry.hpp"
#include "logger/test_sink.hpp"

#include <thread>
#include <vector>

namespace {

sim_logger::LogRecord make_record(sim_logger::Level lvl, std::string logger_name) {
  return sim_logger::LogRecord(
      lvl,
      /*sim_time=*/123.0,
      /*met=*/456.0,
      /*wall_time_ns=*/789,
      std::this_thread::get_id(),
      /*file=*/"file.cpp",
      /*line=*/42,
      /*function=*/"func",
      std::move(logger_name),
      /*tags=*/{},
      /*message=*/"hello");
}

} // namespace

TEST_CASE("TestSink is thread-safe for concurrent writes", "[sprint2][sink]") {
  sim_logger::TestSink sink;

  constexpr int kThreads = 8;
  constexpr int kPerThread = 1000;

  std::vector<std::thread> threads;
  threads.reserve(kThreads);
  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([&sink]() {
      for (int i = 0; i < kPerThread; ++i) {
        sink.write(make_record(sim_logger::Level::Info, "t"));
      }
    });
  }
  for (auto& th : threads) {
    th.join();
  }

  REQUIRE(sink.size() == static_cast<std::size_t>(kThreads * kPerThread));
}

TEST_CASE("Hierarchical loggers inherit level and sinks", "[sprint2][registry]") {
  auto& reg = sim_logger::LoggerRegistry::instance();
  reg.clear();

  auto parent = reg.get_logger("vehicle1");
  auto sink = std::make_shared<sim_logger::TestSink>();
  parent->add_sink(sink);
  parent->set_level(sim_logger::Level::Warn);

  auto child = reg.get_logger("vehicle1.propulsion");
  REQUIRE(child->parent() == parent);
  REQUIRE(child->effective_level() == sim_logger::Level::Warn);
  REQUIRE(child->effective_sinks().size() == 1);

  // Below threshold -> suppressed.
  child->log(make_record(sim_logger::Level::Info, "vehicle1.propulsion"));
  REQUIRE(sink->size() == 0);

  // At/above threshold -> emitted.
  child->log(make_record(sim_logger::Level::Error, "vehicle1.propulsion"));
  REQUIRE(sink->size() == 1);
}

TEST_CASE("Child overrides do not affect parent", "[sprint2][registry]") {
  auto& reg = sim_logger::LoggerRegistry::instance();
  reg.clear();

  auto parent = reg.get_logger("vehicle1");
  auto sink = std::make_shared<sim_logger::TestSink>();
  parent->add_sink(sink);
  parent->set_level(sim_logger::Level::Warn);

  auto child = reg.get_logger("vehicle1.propulsion");

  child->set_level(sim_logger::Level::Debug);
  REQUIRE(child->effective_level() == sim_logger::Level::Debug);
  REQUIRE(parent->effective_level() == sim_logger::Level::Warn);

  // Child now allows INFO.
  child->log(make_record(sim_logger::Level::Info, "vehicle1.propulsion"));
  REQUIRE(sink->size() == 1);

  // Parent still suppresses INFO.
  parent->log(make_record(sim_logger::Level::Info, "vehicle1"));
  REQUIRE(sink->size() == 1);
}

TEST_CASE("Sinks are inherited dynamically when not overridden", "[sprint2][registry]") {
  auto& reg = sim_logger::LoggerRegistry::instance();
  reg.clear();

  auto parent = reg.get_logger("vehicle1");
  auto sink1 = std::make_shared<sim_logger::TestSink>();
  parent->add_sink(sink1);
  parent->set_level(sim_logger::Level::Info);

  auto child = reg.get_logger("vehicle1.propulsion");
  REQUIRE(child->effective_sinks().size() == 1);

  // Add a second sink after the child exists.
  auto sink2 = std::make_shared<sim_logger::TestSink>();
  parent->add_sink(sink2);

  child->log(make_record(sim_logger::Level::Error, "vehicle1.propulsion"));
  REQUIRE(sink1->size() == 1);
  REQUIRE(sink2->size() == 1);
}

