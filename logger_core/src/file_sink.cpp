#include "logger/file_sink.hpp"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#if !defined(_WIN32)
#include <unistd.h>  // fsync, fileno
#endif

namespace sim_logger {
namespace {

std::uint64_t file_size_or_zero(std::FILE* f) {
  if (f == nullptr) {
    return 0;
  }
  // Best-effort: use ftell on an append-opened file. This is used only for
  // rotation accounting and is not required to be perfect on all platforms.
  if (std::fseek(f, 0, SEEK_END) != 0) {
    return 0;
  }
  const long pos = std::ftell(f);
  if (pos < 0) {
    return 0;
  }
  return static_cast<std::uint64_t>(pos);
}

void write_all_or_throw(std::FILE* f, const char* data, size_t size) {
  if (f == nullptr) {
    throw std::runtime_error("FileSink file handle is null");
  }
  const size_t written = std::fwrite(data, 1U, size, f);
  if (written != size) {
    const int err = errno;
    throw std::runtime_error(std::string("FileSink write failed: ") + std::strerror(err));
  }
}

void fflush_or_throw(std::FILE* f) {
  if (f == nullptr) {
    throw std::runtime_error("FileSink file handle is null");
  }
  if (std::fflush(f) != 0) {
    const int err = errno;
    throw std::runtime_error(std::string("FileSink fflush failed: ") + std::strerror(err));
  }
}

#if !defined(_WIN32)
void fsync_or_throw(std::FILE* f) {
  const int fd = ::fileno(f);
  if (fd < 0) {
    const int err = errno;
    throw std::runtime_error(std::string("FileSink fileno failed: ") + std::strerror(err));
  }
  if (::fsync(fd) != 0) {
    const int err = errno;
    throw std::runtime_error(std::string("FileSink fsync failed: ") + std::strerror(err));
  }
}
#endif

}  // namespace

FileSink::FileSink(std::string path, PatternFormatter formatter, bool durable_flush)
    : path_(std::move(path)),
      formatter_(std::move(formatter)),
      durable_flush_(durable_flush) {
  if (path_.empty()) {
    throw std::invalid_argument("FileSink path must not be empty");
  }
  open_or_throw();
}

FileSink::~FileSink() { close_noexcept(); }

void FileSink::open_or_throw() {
  std::lock_guard<std::mutex> lock(mu_);
  open_locked_or_throw();
}

void FileSink::close_noexcept() noexcept {
  std::lock_guard<std::mutex> lock(mu_);
  close_locked_noexcept();
}

void FileSink::write(const LogRecord& record) {
  const std::string line = format_record(record);

  std::lock_guard<std::mutex> lock(mu_);
  write_line_locked(line);
}

std::string FileSink::format_record(const LogRecord& record) const {
  return formatter_.format(record);
}

void FileSink::flush() {
  std::lock_guard<std::mutex> lock(mu_);
  flush_locked();
}

void FileSink::write_line_locked(std::string_view line) {
  write_all_or_throw(file_, line.data(), line.size());
  bytes_written_.fetch_add(static_cast<std::uint64_t>(line.size()), std::memory_order_relaxed);

  if (line.empty() || line.back() != '\n') {
    write_all_or_throw(file_, "\n", 1U);
    bytes_written_.fetch_add(1U, std::memory_order_relaxed);
  }
}

void FileSink::flush_locked() {
  fflush_or_throw(file_);

#if !defined(_WIN32)
  if (durable_flush_) {
    fsync_or_throw(file_);
  }
#else
  (void)durable_flush_;
#endif
}

void FileSink::close_locked_noexcept() noexcept {
  if (file_ != nullptr) {
    std::fclose(file_);
    file_ = nullptr;
  }
  bytes_written_.store(0, std::memory_order_relaxed);
}

void FileSink::open_locked_or_throw() {
  // Append mode; create if missing.
  file_ = std::fopen(path_.c_str(), "a");
  if (file_ == nullptr) {
    const int err = errno;
    throw std::runtime_error(std::string("FileSink fopen failed for '") + path_ +
                             "': " + std::strerror(err));
  }
  bytes_written_.store(file_size_or_zero(file_), std::memory_order_relaxed);
}

void FileSink::reopen_locked(std::string_view new_path) {
  close_locked_noexcept();
  path_ = std::string(new_path);
  open_locked_or_throw();
}

std::uint64_t FileSink::bytes_written() const noexcept {
  return bytes_written_.load(std::memory_order_relaxed);
}

}  // namespace sim_logger
