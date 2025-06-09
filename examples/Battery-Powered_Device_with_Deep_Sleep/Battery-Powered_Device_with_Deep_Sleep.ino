#include <AViShaOTA.h>

#define WAKE_PIN 33
#define BATTERY_PIN 35

AViShaOTA ota("battery-sensor");
RTC_DATA_ATTR int bootCount = 0;

void checkBattery() {
  int batteryLevel = analogRead(BATTERY_PIN);
  float voltage = (batteryLevel * 3.3) / 4095.0 * 2; // Voltage divider
  
  Serial.printf("Battery: %.2fV\n", voltage);
  
  if (voltage < 3.2) {
    Serial.println("Low battery - entering deep sleep");
    esp_deep_sleep_start();
  }
}

void setup() {
  Serial.begin(115200);
  bootCount++;
  
  Serial.printf("Boot count: %d\n", bootCount);
  
  // Check if we should enter OTA mode
  pinMode(WAKE_PIN, INPUT_PULLUP);
  bool otaMode = (digitalRead(WAKE_PIN) == LOW);
  
  if (otaMode || bootCount == 1) {
    Serial.println("Entering OTA mode...");
    
    ota.setOTAPassword("battery123");
    ota.enableSerialDebug(true);
    
    ota.onWiFiConnected([]() {
      Serial.println("OTA Mode Active!");
      Serial.println("URL: " + ota.getUploadURL());
      Serial.println("Hold WAKE pin LOW to exit OTA mode");
    });
    
    if (ota.begin("Battery_Network", "battery_pass")) {
      Serial.println("OTA service started");
      
      // Stay in OTA mode for 5 minutes or until wake pin released
      unsigned long otaStart = millis();
      while (millis() - otaStart < 300000) { // 5 minutes
        ota.handle();
        
        if (digitalRead(WAKE_PIN) == HIGH) {
          Serial.println("Exiting OTA mode...");
          break;
        }
        
        delay(100);
      }
    }
  }
  
  // Normal operation
  checkBattery();
  
  // Do sensor reading and data transmission
  Serial.println("Taking sensor reading...");
  // Your sensor code here...
  
  // Sleep for 1 hour
  Serial.println("Going to sleep for 1 hour...");
  esp_sleep_enable_timer_wakeup(3600 * 1000000ULL); // 1 hour in microseconds
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0); // Wake on WAKE_PIN LOW
  esp_deep_sleep_start();
}

void loop() {
  // This will never be reached due to deep sleep
}
