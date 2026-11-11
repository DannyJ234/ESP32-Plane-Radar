#pragma once

namespace ui::hourly {

/** Turn the on-the-hour photos on/off. */
inline constexpr bool kEnabled = true;

/** How long each photo stays up when the hour changes (ms). */
inline constexpr unsigned long kHoldMs = 4000;

/** Time zone (POSIX format). This one is the UK: GMT, switching to BST in summer. */
inline constexpr char kTimeZone[] = "GMT0BST,M3.5.0/1,M10.5.0";

/**
 * Quiet hours: no photos from kQuietFromHour up to (not including) kQuietToHour,
 * 24-hour clock. Example 23 and 7 = silent from 11pm to 7am.
 * Leave both the same (0 and 0) to show photos around the clock.
 */
inline constexpr int kQuietFromHour = 0;
inline constexpr int kQuietToHour = 0;

}  // namespace ui::hourly