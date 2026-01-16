#pragma once

#include "logger/level.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <thread>

namespace sim_logger {

/**
 * @file log_record.hpp
 * @brief Defines the immutable data model for a single log event.
 *
 * @details
 * A LogRecord represents a *fully materialized* log entry after all metadata
 * has been collected (time, severity, source location, etc.).
 *
 * Design goals:
 *  - Immutable after construction (safe to pass across threads)
 *  - Self-contained (no dangling references)
 *  - Minimal surprise for simulation engineers reading logs
 *
 * LogRecord is intentionally a *data object only*:
 *  - no formatting logic
 *  - no filtering logic
 *  - no sink logic
 *
 * Those responsibilities are handled elsewhere in the pipeline.
 */

/**
 * @brief Simple key/value tag attached to a log record.
 *
 * @details
 * Tags allow log messages to be annotated with contextual information
 * such as vehicle ID, subsystem, or scenario.
 *
 * Example:
 *   subsystem=GNC
 *   vehicle=2
 */
struct Tag {
  std::string key;
  std::string value;
};

/**
 * @brief Immutable representation of a single log entry.
 *
 * @details
 * Each LogRecord contains:
 *  - Time information (simulation, MET, host timestamp)
 *  - Severity level
 *  - Thread identity
 *  - Source location (file, line, function)
 *  - Logger identity (hierarchical name)
 *  - Optional tags
 *  - Final message text
 *
 * Once constructed, a LogRecord must not be modified.
 * This simplifies reasoning about concurrency and asynchronous logging.
 */
class LogRecord {
 public:
  /**
   * @brief Construct a fully populated log record.
   *
   * @param level Severity of the message.
   * @param sim_time Simulation time (seconds).
   * @param met Mission elapsed time (seconds).
   * @param wall_time_ns Monotonic host timestamp (nanoseconds).
   * @param thread_id Thread identifier of the calling thread.
   * @param file Source file name.
   * @param line Source line number.
   * @param function Function name.
   * @param logger_name Hierarchical logger name (e.g., vehicle1.propulsion).
   * @param tags Optional context tags.
   * @param message Fully formatted message text.
   */
  LogRecord(Level level,
            double sim_time,
            double met,
            int64_t wall_time_ns,
            std::thread::id thread_id,
            std::string file,
            int line,
            std::string function,
            std::string logger_name,
            std::vector<Tag> tags,
            std::string message) noexcept
    : level_(level),
      sim_time_(sim_time),
      met_(met),
      wall_time_ns_(wall_time_ns),
      thread_id_(thread_id),
      file_(std::move(file)),
      line_(line),
      function_(std::move(function)),
      logger_name_(std::move(logger_name)),
      tags_(std::move(tags)),
      message_(std::move(message)) {}

  // --- Accessors (no setters by design) ---

  Level level() const noexcept { return level_; }
  double sim_time() const noexcept { return sim_time_; }
  double mission_elapsed() const noexcept { return met_; }
  int64_t wall_time_ns() const noexcept { return wall_time_ns_; }
  std::thread::id thread_id() const noexcept { return thread_id_; }

  std::string_view file() const noexcept { return file_; }
  int line() const noexcept { return line_; }
  std::string_view function() const noexcept { return function_; }
  std::string_view logger_name() const noexcept { return logger_name_; }

  const std::vector<Tag>& tags() const noexcept { return tags_; }
  std::string_view message() const noexcept { return message_; }

 private:
  Level level_;
  double sim_time_;
  double met_;
  int64_t wall_time_ns_;
  std::thread::id thread_id_;

  std::string file_;
  int line_;
  std::string function_;
  std::string logger_name_;
  std::vector<Tag> tags_;
  std::string message_;
};

} // namespace sim_logger
