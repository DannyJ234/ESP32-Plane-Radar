#pragma once

namespace ui {

/** Start time sync (call once in setup(), after Wi-Fi settings are loaded). */
void hourlyPhotosBegin();

/** True once, just after the local hour changes (needs a synced clock). */
bool hourlyPhotosDue();

/** Play the photos (blocks ~4 s per photo). Redraw the radar afterwards. */
void hourlyPhotosRun();

}  // namespace ui