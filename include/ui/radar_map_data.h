#pragma once

#include <cstddef>
#include <cstdint>

namespace ui {

struct RadarMapImage {
  float outer_km;
  const uint8_t* data;
  size_t len;
};

inline constexpr double kRadarMapLat = 51.351246644627366;
inline constexpr double kRadarMapLon = -0.21796408932666123;
inline constexpr size_t kRadarMapCount = 4;
extern const RadarMapImage kRadarMaps[kRadarMapCount];

}  // namespace ui
