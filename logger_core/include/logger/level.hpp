#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace sim_logger {

/**
 * @file level.hpp
 * @brief Defines the severity (log level) model used by the stand-alone logger core.
 *
 * @details
 * This logger is intended to work both:
 *  - Stand-alone (unit tests, reuse across models), and
 *  - Integrated with NASA Trick (via an optional adapter/sink).
 *
 * Industry-standard logging libraries typically keep the core severity model *semantic* and
 * integration-agnostic, and then map levels to external systems (like Trick) in an adapter.
 * We adopt that approach here:
 *
 *  - The core defines semantic severities (Debug/Info/Warn/Error/Fatal).
 *  - The Trick adapter (added later) will map these semantic severities to Trick numeric levels.
 *
 * This separation keeps the core portable and testable while still satisfying Trick compatibility.
 */

/**
 * @brief Semantic, integration-agnostic severity levels.
 *
 * @details
 * The ordering of values is intentional and supports simple threshold filtering:
 * if `lvl >= threshold`, then the message is considered "severe enough" to emit.
 *
 * Example:
 *  - threshold = Warn  -> allow Warn, Error, Fatal (suppress Debug/Info)
 *
 * @note
 * The core does NOT encode Trick numeric values here. Trick mapping is handled by the Trick adapter.
 */
enum class Level : uint8_t {
  Debug = 0,  ///< Diagnostic detail (often high-volume). Usually disabled in production runs.
  Info,       ///< Routine operational messages (low concern).
  Warn,       ///< Unusual but recoverable conditions.
  Error,      ///< Conditions that may affect correctness or require attention.
  Fatal       ///< Unrecoverable conditions; indicates the simulation/run should terminate.
};

/**
 * @brief Convert a severity level to a canonical uppercase string.
 *
 * @param lvl Severity level.
 * @return One of: "DEBUG", "INFO", "WARN", "ERROR", "FATAL".
 *
 * @note
 * This is constexpr and allocation-free to keep overhead minimal.
 */
constexpr std::string_view to_string(Level lvl) noexcept {
  switch (lvl) {
    case Level::Debug: return "DEBUG";
    case Level::Info:  return "INFO";
    case Level::Warn:  return "WARN";
    case Level::Error: return "ERROR";
    case Level::Fatal: return "FATAL";
  }
  return "UNKNOWN";
}

/**
 * @brief Parse a string into a Level (case-insensitive ASCII).
 *
 * @details
 * Supported inputs include:
 *  - "DEBUG"
 *  - "INFO"
 *  - "WARN" or "WARNING"
 *  - "ERROR"
 *  - "FATAL"
 *
 * @param s Input string.
 * @return Parsed Level on success; std::nullopt if unrecognized.
 *
 * @note
 * This function is designed for configuration parsing (JSON, input files, etc.).
 * It avoids locale-dependent behavior and dynamic allocation.
 */
std::optional<Level> level_from_string(std::string_view s) noexcept;

/**
 * @brief Parse numeric inputs for compatibility with Trick-style level conventions.
 *
 * @details
 * Some existing Trick workflows represent levels numerically:
 *  - 0 normal
 *  - 1 info
 *  - 2 warn
 *  - 3 error
 *  - 10 debug
 *
 * The logger core remains semantic, but we accept these numeric values as a
 * *configuration convenience* and translate them into semantic levels.
 *
 * Mapping:
 *  - 0 -> Info   (Trick "normal")
 *  - 1 -> Info
 *  - 2 -> Warn
 *  - 3 -> Error
 *  - 10 -> Debug
 *
 * @param value Numeric level.
 * @return Parsed Level on success; std::nullopt if unsupported.
 *
 * @note
 * This does not make the core "Trick-dependent"; it simply recognizes common inputs.
 * The actual integration mapping (Level -> Trick publish level) belongs in the Trick adapter.
 */
std::optional<Level> level_from_int(int value) noexcept;

/**
 * @brief Convenience helper for threshold checks (inclusive).
 *
 * @param lvl Message level.
 * @param threshold Current threshold (minimum severity to emit).
 * @return true if lvl is at least threshold; otherwise false.
 *
 * @note
 * This works because Level values are ordered from least to most severe.
 */
constexpr bool is_at_least(Level lvl, Level threshold) noexcept {
  return static_cast<uint8_t>(lvl) >= static_cast<uint8_t>(threshold);
}

} // namespace sim_logger
