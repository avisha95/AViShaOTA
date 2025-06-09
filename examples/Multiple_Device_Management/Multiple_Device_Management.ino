#include <AViShaOTA.h>
#include <EEPROM.h>

// Device ID stored in EEPROM
struct DeviceConfig {
  char deviceId[32];
  char otaPassword[32];
  bool isConfigured;
};

AViShaOTA ota;
DeviceConfig config;

void loadConfig() {
  EEPROM.begin(sizeof(DeviceConfig));
  EEPROM.get(0, config);
  
  if (!config.isConfigured) {
    // First time setup
    sprintf(config.deviceId, "device-%06X", ESP.getEfuseMac() & 0xFFFFFF);
    strcpy(config.otaPassword, "default123");
    config.isConfigured = true;
    
    EEPROM.put(0, config);
    EEPROM.commit();
  }
}

void setup() {
  Serial.begin(115200);
  loadConfig();
  
  Serial.printf("Device ID: %s\n", config.deviceId);
  
  // Initialize OTA with unique device ID
  ota = AViShaOTA(config.deviceId, 80);
  ota.setOTAPassword(config.otaPassword);
  
  ota.onWiFiConnected([]() {
    Serial.println("Device online - OTA available at:");
    Serial.println("http://" + ota.getLocalIP() + "/");
    Serial.printf("Device: %s\n", config.deviceId);
  });
  
  // Start with your WiFi credentials
  if (ota.begin("MANAGEMENT_WIFI", "management_pass")) {
    Serial.println("Multi-device OTA system ready!");
  }
}

void loop() {
  ota.handle();
  
  // Device-specific functionality
  // ...
  
  delay(100);
}
