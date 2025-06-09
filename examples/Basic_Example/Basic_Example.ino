#include <AViShaOTA.h>
AViShaOTA ota("avisha-ota");

void setup() {
  Serial.begin(115200);
  ota.setOTAPassword("admin123");
  ota.begin("wifi_name", "wifi_password");
}

void loop() {
  ota.handle();
}
