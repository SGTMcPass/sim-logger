#include "logger/rotating_file_sink.hpp"

#include <cerrno>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>
#include <stdexcept>

#if !defined(_WIN32)
#include <ctime>
#else
#include <time.h>
#endif

namespace sim_logger {
namespace {

namespace fs = std::filesystem;

bool file_exists(const std::string& path) {
  std::error_code ec;
  return fs::exists(fs::path(path), ec);
}

void rename_or_throw(const std::string& from, const std::string& to) {
  if (std::rename(from.c_str(), to.c_str()) != 0) {
    const int err = errno;
    throw std::runtime_error(std::string("RotatingFileSink rename failed: '") + from +
                             "' -> '" + to + "': " + std::strerror(err));
  }
}

}  // namespace

RotatingFileSink::RotatingFileSink(std::string path,
                                   PatternFormatter formatter,
                                   std::uint64_t max_bytes,
                                   bool durable_flush,
                                   std::size_t max_rotated_files)
    : FileSink(path, std::move(formatter), durable_flush),
      base_path_(std::move(path)),
      max_bytes_(max_bytes),
      max_rotated_files_(max_rotated_files) {
  if (base_path_.empty()) {
    throw std::invalid_argument("RotatingFileSink path must not be empty");
  }
  if (max_bytes_ == 0) {
    throw std::invalid_argument("RotatingFileSink max_bytes must be > 0");
  }
}

void RotatingFileSink::write(const LogRecord& record) {
  const std::string line = format_record(record);

  std::lock_guard<std::mutex> lock(mutex());

  const std::uint64_t projected = bytes_written() + static_cast<std::uint64_t>(line.size()) +
                                  ((line.empty() || line.back() != '\n') ? 1U : 0U);

  if (projected >= max_bytes_) {
    rotate_locked();
  }

  write_line_locked(line);
}

void RotatingFileSink::rotate_locked() {
  // Assumes mutex already held.
  flush_locked();
  close_locked_noexcept();

  const std::string ts = make_timestamp_utc();
  const std::string rotated_base = base_path_ + "." + ts;

  // Ensure uniqueness if multiple rotations occur within the same second.
  // POSIX rename() may overwrite existing files, which would be catastrophic.
  std::string rotated_name = rotated_base;
  if (file_exists(rotated_name)) {
    for (std::uint32_t seq = 1; seq < 10000; ++seq) {
      rotated_name = rotated_base + "." + std::to_string(seq);
      if (!file_exists(rotated_name)) {
        break;
      }
    }
    if (file_exists(rotated_name)) {
      throw std::runtime_error("RotatingFileSink: unable to find unique rotated filename");
    }
  }

  rename_or_throw(base_path_, rotated_name);

  // Reopen the base file for continued logging.
  reopen_locked(base_path_);
  ++rotations_performed_;

  prune_old_rotations_locked();
}

void RotatingFileSink::prune_old_rotations_locked() noexcept {
  if (max_rotated_files_ == 0) {
    return;
  }

  try {
    const fs::path base_path(base_path_);
    const fs::path dir = base_path.parent_path().empty() ? fs::path(".") : base_path.parent_path();
    const std::string base_filename = base_path.filename().string();

    struct Candidate {
      fs::path path;
      std::string ts;
      std::uint32_t seq{0};
    };

    std::vector<Candidate> candidates;
    std::error_code ec;
    for (const auto& ent : fs::directory_iterator(dir, ec)) {
      if (ec) {
        break;
      }
      if (!ent.is_regular_file(ec)) {
        continue;
      }

      std::string ts;
      std::uint32_t seq = 0;
      const std::string fn = ent.path().filename().string();
      if (!parse_rotation_suffix(fn, base_filename, &ts, &seq)) {
        continue;
      }
      candidates.push_back(Candidate{ent.path(), std::move(ts), seq});
    }

    if (candidates.size() <= max_rotated_files_) {
      return;
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
      if (a.ts != b.ts) {
        return a.ts < b.ts;
      }
      return a.seq < b.seq;
    });

    const std::size_t to_delete = candidates.size() - max_rotated_files_;
    for (std::size_t i = 0; i < to_delete; ++i) {
      std::error_code rm_ec;
      fs::remove(candidates[i].path, rm_ec);
      // Best-effort: ignore failures.
    }
  } catch (...) {
    // Best-effort: never throw from pruning.
  }
}

bool RotatingFileSink::parse_rotation_suffix(const std::string& filename,
                                             const std::string& base_filename,
                                             std::string* out_ts,
                                             std::uint32_t* out_seq) noexcept {
  // Expected patterns:
  //   <base_filename>.<YYYYMMDD_HHMMSS>
  //   <base_filename>.<YYYYMMDD_HHMMSS>.<seq>
  // We parse the timestamp (lexicographically sortable) and an optional numeric seq.
  const std::string prefix = base_filename + ".";
  if (filename.rfind(prefix, 0) != 0) {
    return false;
  }
  const std::string rest = filename.substr(prefix.size());
  if (rest.size() < 15) {
    return false;
  }
  const std::string ts = rest.substr(0, 15);

  // Validate timestamp shape: 8 digits, underscore, 6 digits.
  for (int i = 0; i < 8; ++i) {
    if (ts[i] < '0' || ts[i] > '9') {
      return false;
    }
  }
  if (ts[8] != '_') {
    return false;
  }
  for (int i = 9; i < 15; ++i) {
    if (ts[i] < '0' || ts[i] > '9') {
      return false;
    }
  }

  std::uint32_t seq = 0;
  if (rest.size() > 15) {
    if (rest[15] != '.') {
      return false;
    }
    const std::string seq_str = rest.substr(16);
    if (seq_str.empty()) {
      return false;
    }
    std::uint32_t acc = 0;
    for (char c : seq_str) {
      if (c < '0' || c > '9') {
        return false;
      }
      acc = static_cast<std::uint32_t>(acc * 10 + static_cast<std::uint32_t>(c - '0'));
    }
    seq = acc;
  }

  if (out_ts) {
    *out_ts = ts;
  }
  if (out_seq) {
    *out_seq = seq;
  }
  return true;
}

std::string RotatingFileSink::make_timestamp_utc() {
  using clock = std::chrono::system_clock;
  const auto now = clock::now();
  const std::time_t t = clock::to_time_t(now);

  std::tm utc_tm{};
#if defined(_WIN32)
  gmtime_s(&utc_tm, &t);
#else
  gmtime_r(&t, &utc_tm);
#endif

  char buf[32] = {0};
  if (std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &utc_tm) == 0) {
    // Extremely unlikely; provide a deterministic fallback.
    return "00000000_000000";
  }
  return std::string(buf);
}

}  // namespace sim_logger
