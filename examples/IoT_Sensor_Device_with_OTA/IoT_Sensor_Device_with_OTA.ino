#include <AViShaOTA.h>
#include <DHT.h>

#define DHT_PIN 2
#define LED_PIN 13

DHT dht(DHT_PIN, DHT22);
AViShaOTA ota("sensor-node");

unsigned long lastSensorRead = 0;
bool otaActive = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  
  // OTA Configuration
  ota.setOTAPassword("sensor123");
  ota.enableSerialDebug(true);
  
  // OTA Callbacks
  ota.onStart([]() {
    otaActive = true;
    Serial.println("OTA Started - Pausing sensor readings");
    digitalWrite(LED_PIN, HIGH); // Indicate OTA mode
  });
  
  ota.onEnd([]() {
    otaActive = false;
    Serial.println("OTA Finished - Resuming normal operation");
    digitalWrite(LED_PIN, LOW);
  });
  
  ota.onProgress([](unsigned int progress, unsigned int total) {
    // Blink LED during update
    digitalWrite(LED_PIN, (millis() % 500) < 250);
  });
  
  // Start OTA
  if (ota.begin("IoT_Network", "network_password")) {
    Serial.println("Sensor Node Online!");
    Serial.println("OTA URL: " + ota.getUploadURL());
  }
}

void loop() {
  ota.handle();
  
  // Only read sensors when not updating
  if (!otaActive && millis() - lastSensorRead > 5000) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    if (!isnan(temperature) && !isnan(humidity)) {
      Serial.printf("Temp: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
      
      // Send to cloud/server here
      // ...
    }
    
    lastSensorRead = millis();
  }
  
  delay(100);
}