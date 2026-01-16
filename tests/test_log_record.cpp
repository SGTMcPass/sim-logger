#include <catch2/catch_test_macros.hpp>

#include "logger/log_record.hpp"

#include <thread>

using namespace sim_logger;

TEST_CASE("LogRecord stores and exposes all provided fields", "[log_record]") {
  std::vector<Tag> tags{
    {"vehicle", "1"},
    {"subsystem", "GNC"}
  };

  const auto thread_id = std::this_thread::get_id();

  LogRecord record(
    /*level=*/Level::Warn,
    /*sim_time=*/123.45,
    /*met=*/67.89,
    /*wall_time_ns=*/9'876'543'210,
    /*thread_id=*/thread_id,
    /*file=*/"example.cpp",
    /*line=*/42,
    /*function=*/"update_guidance",
    /*logger_name=*/"vehicle1.gnc",
    /*tags=*/tags,
    /*message=*/"Guidance solution diverged"
  );

  REQUIRE(record.level() == Level::Warn);
  REQUIRE(record.sim_time() == 123.45);
  REQUIRE(record.mission_elapsed() == 67.89);
  REQUIRE(record.wall_time_ns() == 9'876'543'210);
  REQUIRE(record.thread_id() == thread_id);

  REQUIRE(record.file() == std::string_view{"example.cpp"});
  REQUIRE(record.line() == 42);
  REQUIRE(record.function() == std::string_view{"update_guidance"});
  REQUIRE(record.logger_name() == std::string_view{"vehicle1.gnc"});
  REQUIRE(record.message() == std::string_view{"Guidance solution diverged"});

  REQUIRE(record.tags().size() == 2);
  REQUIRE(record.tags()[0].key == "vehicle");
  REQUIRE(record.tags()[0].value == "1");
  REQUIRE(record.tags()[1].key == "subsystem");
  REQUIRE(record.tags()[1].value == "GNC");
}

TEST_CASE("LogRecord owns its data (no aliasing of constructor arguments)", "[log_record]") {
  std::string file = "temp.cpp";
  std::string function = "temp_function";
  std::string logger_name = "temp.logger";
  std::string message = "temporary message";

  std::vector<Tag> tags{{"key", "value"}};

  LogRecord record(
    Level::Info,
    0.0,
    0.0,
    0,
    std::this_thread::get_id(),
    file,
    1,
    function,
    logger_name,
    tags,
    message
  );

  // Mutate originals
  file.clear();
  function.clear();
  logger_name.clear();
  message.clear();
  tags.clear();

  REQUIRE(record.file() == std::string_view{"temp.cpp"});
  REQUIRE(record.function() == std::string_view{"temp_function"});
  REQUIRE(record.logger_name() == std::string_view{"temp.logger"});
  REQUIRE(record.message() == std::string_view{"temporary message"});

  REQUIRE(record.tags().size() == 1);
  REQUIRE(record.tags()[0].key == "key");
  REQUIRE(record.tags()[0].value == "value");
}
