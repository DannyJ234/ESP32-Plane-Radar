#include "ui/radar_map.h"

#include <Arduino.h>

#include <cmath>

#include "services/radar_location.h"
#include "ui/radar_map_config.h"
#include "ui/radar_map_data.h"
#include "ui/radar_range.h"

namespace ui::radarmap {

namespace {
bool s_timing_logged = false;
}

void draw(lgfx::LGFXBase& gfx) {
  if (!kEnabled) {
    return;
  }
  if (std::fabs(services::location::lat() - kRadarMapLat) > kMaxOffsetDeg ||
      std::fabs(services::location::lon() - kRadarMapLon) > kMaxOffsetDeg) {
    return;  // radar moved: the stored map no longer lines up
  }

  const float outer = radar::rangeCurrent().outer_km;
  for (size_t i = 0; i < kRadarMapCount; ++i) {
    if (std::fabs(kRadarMaps[i].outer_km - outer) <= outer * kRangeTolerance) {
      const unsigned long t0 = micros();
      gfx.drawJpg(kRadarMaps[i].data, kRadarMaps[i].len, 0, 0);
      if (!s_timing_logged) {
        s_timing_logged = true;
        Serial.printf("map: drew %.1f km background in %lu ms\n", outer,
                      (micros() - t0) / 1000UL);
      }
      return;
    }
  }
}

}  // namespace ui::radarmap
