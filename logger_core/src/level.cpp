#include "logger/level.hpp"

namespace sim_logger {

/**
 * @file level.cpp
 * @brief Implements level parsing utilities.
 *
 * @details
 * These utilities are used primarily during configuration parsing.
 * They are intentionally:
 *  - allocation-free,
 *  - ASCII-only (no locale surprises),
 *  - noexcept (logging infrastructure should not throw during normal operation).
 */

namespace {

constexpr char to_upper_ascii(char c) noexcept {
  return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

bool iequals_ascii(std::string_view a, std::string_view b) noexcept {
  if (a.size() != b.size()) {
    return false;
  }
  for (size_t i = 0; i < a.size(); ++i) {
    if (to_upper_ascii(a[i]) != to_upper_ascii(b[i])) {
      return false;
    }
  }
  return true;
}

} // namespace

std::optional<Level> level_from_string(std::string_view s) noexcept {
  if (iequals_ascii(s, "DEBUG")) return Level::Debug;
  if (iequals_ascii(s, "INFO"))  return Level::Info;
  if (iequals_ascii(s, "WARN") || iequals_ascii(s, "WARNING")) return Level::Warn;
  if (iequals_ascii(s, "ERROR")) return Level::Error;
  if (iequals_ascii(s, "FATAL")) return Level::Fatal;

  return std::nullopt;
}

std::optional<Level> level_from_int(int value) noexcept {
  // Numeric parsing is a configuration convenience for Trick-style conventions.
  // The core remains semantic; Trick mapping for publishing is handled in the adapter.
  switch (value) {
    case 0:  // Trick "normal"
    case 1:  // Trick "info"
      return Level::Info;
    case 2:  // Trick "warn"
      return Level::Warn;
    case 3:  // Trick "error"
      return Level::Error;
    case 10: // Trick "debug"
      return Level::Debug;
    default:
      return std::nullopt;
  }
}

} // namespace sim_logger
