#pragma once

#include <LovyanGFX.hpp>

namespace ui::radarmap {

/**
 * Draw the terrain/water/parks background for the current range onto `gfx`.
 * Does nothing if disabled, if the radar has been moved away from the place
 * the map was made for, or if there is no image for the current range.
 * Call right after clearing the screen and before drawing the rings.
 */
void draw(lgfx::LGFXBase& gfx);

}  // namespace ui::radarmap
