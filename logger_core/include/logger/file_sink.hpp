#pragma once

#include "logger/pattern_formatter.hpp"
#include "logger/sink.hpp"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>

namespace sim_logger {

/**
 * @brief Synchronous file sink (append-only). No rotation (Sprint 3).
 *
 * Thread-safety:
 * - Serializes writes and flushes using an internal mutex.
 *
 * Error behavior:
 * - Throws std::runtime_error on open/write/flush failures.
 * - Logger is responsible for exception containment.
 */
class FileSink : public ISink {
 public:
  /**
   * @param path Output file path (opened in append mode).
   * @param formatter Formatter used to render records.
   * @param durable_flush If true, flush() will attempt an fsync() on POSIX.
   */
  FileSink(std::string path, PatternFormatter formatter, bool durable_flush = false);

  ~FileSink() override;

  void write(const LogRecord& record) override;
  void flush() override;

  const std::string& path() const noexcept { return path_; }
  bool durable_flush() const noexcept { return durable_flush_; }

 protected:
  // --- Hooks for derived sinks (e.g., RotatingFileSink) ---
  // These helpers assume the caller already holds mu_.

  void write_line_locked(std::string_view line);
  std::string format_record(const LogRecord& record) const;
  void flush_locked();
  void close_locked_noexcept() noexcept;
  void open_locked_or_throw();

  // Reopen the sink at a (possibly new) path. Primarily intended for rotation.
  void reopen_locked(std::string_view new_path);

  // Bytes written to the current file (best-effort; includes newline written by FileSink).
  std::uint64_t bytes_written() const noexcept;

  // Expose the lock to derived sinks for composing operations safely.
  std::mutex& mutex() noexcept { return mu_; }
  const std::mutex& mutex() const noexcept { return mu_; }

 private:
  std::string path_;
  PatternFormatter formatter_;
  bool durable_flush_{false};

  std::FILE* file_{nullptr};
  mutable std::mutex mu_;
  std::atomic<std::uint64_t> bytes_written_{0};

  void open_or_throw();
  void close_noexcept() noexcept;
};

}  // namespace sim_logger
