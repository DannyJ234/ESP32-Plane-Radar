#include "ui/hourly_photos.h"

#include <Arduino.h>
#include <time.h>

#include "ui/hourly_photos_config.h"
#include "ui/hourly_photos_logic.h"
#include "ui/photo_splash.h"

namespace ui {
namespace {

long s_last_key = -1;
unsigned long s_last_check_ms = 0;
bool s_began = false;

}  // namespace

void hourlyPhotosBegin() {
  if (!hourly::kEnabled || s_began) return;
  s_began = true;
  // Starts background time sync; it completes once Wi-Fi is up.
  configTzTime(hourly::kTimeZone, "pool.ntp.org", "time.google.com");
}

bool hourlyPhotosDue() {
  if (!hourly::kEnabled || !s_began) return false;
  const unsigned long now_ms = millis();
  if (now_ms - s_last_check_ms < 500) return false;
  s_last_check_ms = now_ms;

  const time_t now = time(nullptr);
  if (now < 1700000000) return false;  // clock not synced yet
  struct tm local;
  localtime_r(&now, &local);
  return hourly::hourJustChanged(local, s_last_key, hourly::kQuietFromHour,
                                 hourly::kQuietToHour);
}

void hourlyPhotosRun() {
  Serial.println("hourly photos");
  photoSlideshow(hourly::kHoldMs);
}

}  // namespace ui