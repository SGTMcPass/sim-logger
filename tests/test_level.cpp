#include <catch2/catch_test_macros.hpp>

#include "logger/level.hpp"

#include <string_view>

using sim_logger::Level;

TEST_CASE("Level::to_string returns canonical uppercase names", "[level]") {
  REQUIRE(sim_logger::to_string(Level::Debug) == std::string_view{"DEBUG"});
  REQUIRE(sim_logger::to_string(Level::Info)  == std::string_view{"INFO"});
  REQUIRE(sim_logger::to_string(Level::Warn)  == std::string_view{"WARN"});
  REQUIRE(sim_logger::to_string(Level::Error) == std::string_view{"ERROR"});
  REQUIRE(sim_logger::to_string(Level::Fatal) == std::string_view{"FATAL"});
}

TEST_CASE("level_from_string parses canonical names (case-insensitive)", "[level]") {
  REQUIRE(sim_logger::level_from_string("DEBUG").has_value());
  REQUIRE(sim_logger::level_from_string("debug") == Level::Debug);

  REQUIRE(sim_logger::level_from_string("INFO") == Level::Info);
  REQUIRE(sim_logger::level_from_string("info") == Level::Info);

  REQUIRE(sim_logger::level_from_string("WARN") == Level::Warn);
  REQUIRE(sim_logger::level_from_string("warning") == Level::Warn);
  REQUIRE(sim_logger::level_from_string("WaRn") == Level::Warn);

  REQUIRE(sim_logger::level_from_string("ERROR") == Level::Error);
  REQUIRE(sim_logger::level_from_string("error") == Level::Error);

  REQUIRE(sim_logger::level_from_string("FATAL") == Level::Fatal);
  REQUIRE(sim_logger::level_from_string("fatal") == Level::Fatal);
}

TEST_CASE("level_from_string rejects unknown strings", "[level]") {
  REQUIRE_FALSE(sim_logger::level_from_string("").has_value());
  REQUIRE_FALSE(sim_logger::level_from_string("VERBOSE").has_value());
  REQUIRE_FALSE(sim_logger::level_from_string("TRACE").has_value()); // not supported in this build
}

TEST_CASE("level_from_int supports Trick-style numeric inputs (compatibility)", "[level]") {
  // Trick "normal" and "info" -> Info
  REQUIRE(sim_logger::level_from_int(0) == Level::Info);
  REQUIRE(sim_logger::level_from_int(1) == Level::Info);

  // Trick warn/error/debug
  REQUIRE(sim_logger::level_from_int(2) == Level::Warn);
  REQUIRE(sim_logger::level_from_int(3) == Level::Error);
  REQUIRE(sim_logger::level_from_int(10) == Level::Debug);
}

TEST_CASE("level_from_int rejects unsupported numeric inputs", "[level]") {
  REQUIRE_FALSE(sim_logger::level_from_int(-1).has_value());
  REQUIRE_FALSE(sim_logger::level_from_int(4).has_value());
  REQUIRE_FALSE(sim_logger::level_from_int(9).has_value());
  REQUIRE_FALSE(sim_logger::level_from_int(11).has_value());
}

TEST_CASE("is_at_least implements inclusive threshold semantics", "[level]") {
  REQUIRE(sim_logger::is_at_least(Level::Warn, Level::Warn));
  REQUIRE(sim_logger::is_at_least(Level::Error, Level::Warn));
  REQUIRE(sim_logger::is_at_least(Level::Fatal, Level::Warn));

  REQUIRE_FALSE(sim_logger::is_at_least(Level::Info, Level::Warn));
  REQUIRE_FALSE(sim_logger::is_at_least(Level::Debug, Level::Warn));
}
