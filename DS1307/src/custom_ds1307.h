#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
#include <Arduino.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))



void rtc_setup();
void getCurrentTime();
void printDateTime(const RtcDateTime);
