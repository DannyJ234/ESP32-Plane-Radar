#pragma once

namespace ui::radarmap {

/** Switch the map background on/off. */
inline constexpr bool kEnabled = true;

/** The map is only drawn while the radar centre is within this many degrees
 *  of the place it was generated for (0.003 deg is roughly 300 m). */
inline constexpr double kMaxOffsetDeg = 0.003;

/** A range matches a map image when its outer ring is within this fraction. */
inline constexpr float kRangeTolerance = 0.03f;

}  // namespace ui::radarmap
