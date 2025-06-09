#include <AViShaOTA.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
AViShaOTA ota("smart-switch");

String deviceStatus = "Ready";
unsigned long lastStatusUpdate = 0;

void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Switch");
  lcd.setCursor(0, 1);
  
  if (ota.isOTAInProgress()) {
    lcd.print("OTA Updating...");
  } else if (ota.isWebUpdateInProgress()) {
    lcd.print("Web Updating...");
  } else if (ota.isConnected()) {
    lcd.print("IP:" + ota.getLocalIP().substring(10));
  } else {
    lcd.print("No WiFi");
  }
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  
  // Show startup message
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  
  // OTA Setup
  ota.setOTAPassword("home123");
  
  ota.onStart([]() {
    deviceStatus = "OTA Update";
    updateDisplay();
  });
  
  ota.onProgress([](unsigned int progress, unsigned int total) {
    lcd.setCursor(0, 1);
    lcd.printf("Progress: %u%%", (progress * 100) / total);
  });
  
  ota.onEnd([]() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Update Complete");
    lcd.setCursor(0, 1);
    lcd.print("Restarting...");
  });
  
  ota.onWiFiConnected([]() {
    deviceStatus = "Connected";
    updateDisplay();
  });
  
  ota.onWiFiDisconnected([]() {
    deviceStatus = "Disconnected";
    updateDisplay();
  });
  
  // Start OTA
  if (ota.begin("SmartHome_WiFi", "home_password")) {
    deviceStatus = "OTA Ready";
    Serial.println("Smart Switch Online!");
  } else {
    deviceStatus = "WiFi Failed";
  }
  
  updateDisplay();
}

void loop() {
  ota.handle();
  
  // Update display every 5 seconds
  if (millis() - lastStatusUpdate > 5000) {
    if (!ota.isOTAInProgress() && !ota.isWebUpdateInProgress()) {
      updateDisplay();
    }
    lastStatusUpdate = millis();
  }
  
  // Your smart switch logic here
  // Button handling, relay control, etc.
  
  delay(100);
}
