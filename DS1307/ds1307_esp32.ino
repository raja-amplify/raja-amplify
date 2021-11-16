#include "src/custom_ds1307.h"
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  rtc_setup();

}

void loop() {
  // put your main code here, to run repeatedly:

  getCurrentTime();

  delay(5000);

}
