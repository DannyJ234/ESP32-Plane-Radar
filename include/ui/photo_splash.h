#pragma once

namespace ui {

/**
 * Startup photo slideshow with a curved caption along the top edge. Blocks
 * until it has finished, leaving the screen black. Call right after
 * displayInit().
 */
void photoSplashShow();

/**
 * Play the photos over whatever is on screen, `hold_ms` each, then close to
 * black. Blocks until finished; the caller must redraw the radar afterwards.
 */
void photoSlideshow(unsigned long hold_ms);

}  // namespace ui