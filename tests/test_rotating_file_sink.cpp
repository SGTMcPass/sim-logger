#include <catch2/catch_test_macros.hpp>

#include "logger/rotating_file_sink.hpp"

#include "logger/log_record.hpp"
#include "logger/pattern_formatter.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

namespace {

std::vector<std::string> read_all_lines(const fs::path& p) {
  std::ifstream in(p);
  REQUIRE(in.is_open());

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line)) {
    lines.push_back(line);
  }
  return lines;
}

std::vector<fs::path> list_rotated_files(const fs::path& base) {
  std::vector<fs::path> out;
  const fs::path dir = base.parent_path();
  const std::string base_name = base.filename().string();

  for (const auto& ent : fs::directory_iterator(dir)) {
    if (!ent.is_regular_file()) {
      continue;
    }
    const std::string fn = ent.path().filename().string();
    if (fn.rfind(base_name + ".", 0) == 0) {
      out.push_back(ent.path());
    }
  }
  return out;
}

}  // namespace

TEST_CASE("RotatingFileSink rotates at size threshold and preserves messages") {
  const fs::path tmp = fs::temp_directory_path() / fs::path("sim_logger_rotation_test");
  fs::create_directories(tmp);

  const fs::path base = tmp / fs::path("rotation.log");
  if (fs::exists(base)) {
    fs::remove(base);
  }
  for (const auto& p : list_rotated_files(base)) {
    fs::remove(p);
  }

  // Simple, deterministic formatting: message only.
  sim_logger::PatternFormatter fmt("{msg}");

  // Make the threshold small so rotation triggers quickly.
  sim_logger::RotatingFileSink sink(base.string(), fmt, /*max_bytes=*/40, /*durable_flush=*/false,
                                    /*max_rotated_files=*/0);

  auto make_record = [](std::string msg) {
    return sim_logger::LogRecord(sim_logger::Level::Info,
                                 /*sim_time=*/0.0,
                                 /*met=*/0.0,
                                 /*wall_time_ns=*/0,
                                 std::this_thread::get_id(),
                                 "file.cpp",
                                 123,
                                 "func",
                                 "logger",
                                 {},
                                 std::move(msg));
  };

  const std::vector<std::string> msgs = {
      "id=0001 abcdef",
      "id=0002 abcdef",
      "id=0003 abcdef",
      "id=0004 abcdef",
  };

  for (const auto& m : msgs) {
    sink.write(make_record(m));
  }
  sink.flush();

  const auto rotated = list_rotated_files(base);
  REQUIRE(rotated.size() >= 1);
  REQUIRE(sink.rotations_performed() >= 1);

  // Verify rotated filename format: base.log.YYYYMMDD_HHMMSS[.seq]
  const std::regex re("rotation\\.log\\.[0-9]{8}_[0-9]{6}(\\.[0-9]+)?");
  bool matched = false;
  for (const auto& p : rotated) {
    if (std::regex_match(p.filename().string(), re)) {
      matched = true;
      break;
    }
  }
  REQUIRE(matched);

  // Read all lines from base + rotated, verify all IDs appear exactly once.
  std::vector<std::string> all_lines;
  if (fs::exists(base)) {
    const auto lines = read_all_lines(base);
    all_lines.insert(all_lines.end(), lines.begin(), lines.end());
  }
  for (const auto& p : rotated) {
    const auto lines = read_all_lines(p);
    all_lines.insert(all_lines.end(), lines.begin(), lines.end());
  }

  for (const auto& m : msgs) {
    int count = 0;
    for (const auto& line : all_lines) {
      if (line.find(m) != std::string::npos) {
        ++count;
      }
    }
    REQUIRE(count == 1);
  }
}

TEST_CASE("RotatingFileSink prunes oldest rotated files by filename timestamp after rotation") {
  const fs::path tmp = fs::temp_directory_path() / fs::path("sim_logger_rotation_retention_test");
  fs::create_directories(tmp);

  const fs::path base = tmp / fs::path("retention.log");
  if (fs::exists(base)) {
    fs::remove(base);
  }
  for (const auto& p : list_rotated_files(base)) {
    fs::remove(p);
  }

  sim_logger::PatternFormatter fmt("{msg}");

  // Set a very small threshold to force multiple rotations quickly.
  sim_logger::RotatingFileSink sink(base.string(), fmt, /*max_bytes=*/32, /*durable_flush=*/false,
                                    /*max_rotated_files=*/2);

  auto make_record = [](std::string msg) {
    return sim_logger::LogRecord(sim_logger::Level::Info,
                                 /*sim_time=*/0.0,
                                 /*met=*/0.0,
                                 /*wall_time_ns=*/0,
                                 std::this_thread::get_id(),
                                 "file.cpp",
                                 123,
                                 "func",
                                 "logger",
                                 {},
                                 std::move(msg));
  };

  // Long messages to trigger rotation often.
  for (int i = 0; i < 20; ++i) {
    sink.write(make_record("msg-" + std::to_string(i) + " xxxxxxxxxxxxxxxx"));
  }
  sink.flush();

  const auto rotated = list_rotated_files(base);
  REQUIRE(sink.rotations_performed() >= 2);
  REQUIRE(rotated.size() == 2);
}

TEST_CASE("RotatingFileSink prunes only after a successful rotation") {
  const fs::path tmp = fs::temp_directory_path() / fs::path("sim_logger_rotation_prune_after_test");
  fs::create_directories(tmp);

  const fs::path base = tmp / fs::path("prune_after.log");
  if (fs::exists(base)) {
    fs::remove(base);
  }
  for (const auto& p : list_rotated_files(base)) {
    fs::remove(p);
  }

  // Create several pre-existing rotated files (older timestamps).
  const std::vector<std::string> old_names = {
      "prune_after.log.20000101_000000",
      "prune_after.log.20000101_000001",
      "prune_after.log.20000101_000002",
  };
  for (const auto& n : old_names) {
    std::ofstream out(tmp / fs::path(n));
    out << "old" << std::endl;
  }

  sim_logger::PatternFormatter fmt("{msg}");

  // Threshold large enough that this write does NOT rotate.
  sim_logger::RotatingFileSink no_rotate(base.string(), fmt, /*max_bytes=*/1024 * 1024,
                                         /*durable_flush=*/false, /*max_rotated_files=*/1);

  auto make_record = [](std::string msg) {
    return sim_logger::LogRecord(sim_logger::Level::Info,
                                 /*sim_time=*/0.0,
                                 /*met=*/0.0,
                                 /*wall_time_ns=*/0,
                                 std::this_thread::get_id(),
                                 "file.cpp",
                                 123,
                                 "func",
                                 "logger",
                                 {},
                                 std::move(msg));
  };

  no_rotate.write(make_record("does-not-rotate"));
  no_rotate.flush();

  // Still present because pruning happens only after rotation.
  REQUIRE(list_rotated_files(base).size() == old_names.size());

  // Now force a rotation; pruning should occur after this rotation.
  sim_logger::RotatingFileSink rotate(base.string(), fmt, /*max_bytes=*/32, /*durable_flush=*/false,
                                      /*max_rotated_files=*/1);
  for (int i = 0; i < 20; ++i) {
    rotate.write(make_record("rotate-" + std::to_string(i) + " xxxxxxxxxxxxxxxx"));
  }
  rotate.flush();

  REQUIRE(list_rotated_files(base).size() == 1);
}
