#pragma once

// Pure time logic (no Arduino dependencies) so it can be tested on a computer.

#include <ctime>

namespace ui::hourly {

/** One number per local calendar hour; changes when the hour changes. */
inline long hourKey(const struct tm& t) {
  return (static_cast<long>(t.tm_year) * 366L + t.tm_yday) * 24L + t.tm_hour;
}

inline bool inQuietHours(int hour, int from, int to) {
  if (from == to) return false;
  if (from < to) return hour >= from && hour < to;
  return hour >= from || hour < to;  // wraps past midnight
}

/**
 * Call with the current local time. `last_key` starts at -1. Returns true when
 * a new hour has just begun. The first call only records the hour (so nothing
 * shows right at boot).
 */
inline bool hourJustChanged(const struct tm& now, long& last_key, int quiet_from,
                            int quiet_to) {
  const long key = hourKey(now);
  if (last_key < 0) {
    last_key = key;
    return false;
  }
  if (key == last_key) return false;
  last_key = key;
  return !inQuietHours(now.tm_hour, quiet_from, quiet_to);
}

}  // namespace ui::hourly