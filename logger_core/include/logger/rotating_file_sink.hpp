#pragma once

#include "logger/file_sink.hpp"
#include "logger/pattern_formatter.hpp"

#include <cstdint>
#include <cstddef>
#include <string>

namespace sim_logger {

/**
 * @brief Synchronous file sink with size-based rotation.
 *
 * Rotation is performed as:
 *   flush -> close -> rename-with-timestamp -> reopen base file
 *
 * Sprint 4.0 scope:
 * - Size-based rotation only.
 * - Rename-with-timestamp is required.
 * - Compression/zip is out-of-scope.
 */
class RotatingFileSink final : public FileSink {
 public:
  /**
   * @param path Base output file path (opened in append mode).
   * @param formatter Formatter used to render records.
   * @param max_bytes Rotation threshold. Rotation occurs before writing the line that would
   *        cause the file to reach or exceed max_bytes.
   * @param durable_flush If true, flush() will attempt an fsync() on POSIX.
   * @param max_rotated_files Retention limit (0 = unlimited). When > 0, the sink will prune
   *        oldest rotated files (based on timestamp embedded in filename) after each successful
   *        rotation.
   */
  RotatingFileSink(std::string path,
                   PatternFormatter formatter,
                   std::uint64_t max_bytes,
                   bool durable_flush = false,
                   std::size_t max_rotated_files = 0);

  ~RotatingFileSink() override = default;

  void write(const LogRecord& record) override;

  std::uint64_t max_bytes() const noexcept { return max_bytes_; }
  std::uint64_t rotations_performed() const noexcept { return rotations_performed_; }
  std::size_t max_rotated_files() const noexcept { return max_rotated_files_; }

 private:
  std::string base_path_;
  std::uint64_t max_bytes_{0};
  std::uint64_t rotations_performed_{0};
  std::size_t max_rotated_files_{0};

  void rotate_locked();
  void prune_old_rotations_locked() noexcept;
  static bool parse_rotation_suffix(const std::string& filename,
                                    const std::string& base_filename,
                                    std::string* out_ts,
                                    std::uint32_t* out_seq) noexcept;
  static std::string make_timestamp_utc();
};

}  // namespace sim_logger
