#include <AViShaOTA.h>

AViShaOTA ota("smart-device", 8080);

void setup() {
  Serial.begin(115200);
  
  // Configuration
  ota.setOTAPassword("secure123");
  ota.enableSerialDebug(true);
  ota.enableMDNS(true);
  
  // OTA Callbacks
  ota.onStart([]() {
    Serial.println("OTA Update Started!");
    // Turn on LED or display message
  });
  
  ota.onEnd([]() {
    Serial.println("OTA Update Finished!");
    // Turn off LED
  });
  
  ota.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\n", (progress * 100) / total);
    // Update progress bar on display
  });
  
  ota.onError([](ota_error_t error) {
    Serial.printf("OTA Error: %u\n", error);
    // Handle error - maybe blink LED
  });
  
  // WiFi Callbacks
  ota.onWiFiConnected([]() {
    Serial.println("WiFi Connected - OTA Ready!");
  });
  
  ota.onWiFiDisconnected([]() {
    Serial.println("WiFi Disconnected - OTA Not Available");
  });
  
  // Web Update Callbacks
  ota.onWebUpdateStart([]() {
    Serial.println("Web Update Started!");
  });
  
  ota.onWebUpdateEnd([](bool success) {
    if (success) {
      Serial.println("Web Update Successful!");
    } else {
      Serial.println("Web Update Failed!");
    }
  });
  
  // Start OTA
  if (ota.begin("YourWiFi", "YourPassword")) {
    Serial.println("OTA Service Started Successfully!");
    Serial.println("Upload URL: " + ota.getUploadURL());
  } else {
    Serial.println("Failed to start OTA service!");
  }
}

void loop() {
  ota.handle();
  
  // Your main application code here
  // Example: sensor reading, LED control, etc.
  delay(100);
}