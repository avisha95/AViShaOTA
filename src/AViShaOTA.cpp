// AViShaOTA.cpp - Fixed Password Validation
#include "AViShaOTA.h"

// Static instance pointer
AViShaOTA* AViShaOTA::instance = nullptr;

// Constructor
AViShaOTA::AViShaOTA(const String& hostname, int port) {
  this->hostname = hostname;
  this->serverPort = port;
  this->server = nullptr;
  this->otaPassword = "";
  this->mdnsEnabled = true;
  this->serialDebug = true;
  this->isInitialized = false;
  this->otaInProgress = false;
  this->webUpdateInProgress = false;
  
  // Initialize callbacks to nullptr
  this->onStartCallback = nullptr;
  this->onEndCallback = nullptr;
  this->onProgressCallback = nullptr;
  this->onErrorCallback = nullptr;
  this->onWiFiConnectedCallback = nullptr;
  this->onWiFiDisconnectedCallback = nullptr;
  this->onWebUpdateStartCallback = nullptr;
  this->onWebUpdateEndCallback = nullptr;

  // Set static instance
  instance = this;
}

// Destructor
AViShaOTA::~AViShaOTA() {
  if (server) {
    delete server;
    server = nullptr;
  }
  instance = nullptr;
}

// Configuration methods
void AViShaOTA::setOTAPassword(const String& password) {
  this->otaPassword = password;
}

void AViShaOTA::setHostname(const String& name) {
  this->hostname = name;
}

void AViShaOTA::enableMDNS(bool enable) {
  this->mdnsEnabled = enable;
}

void AViShaOTA::enableSerialDebug(bool enable) {
  this->serialDebug = enable;
}

// Callback setters
void AViShaOTA::onStart(void (*callback)()) {
  this->onStartCallback = callback;
}

void AViShaOTA::onEnd(void (*callback)()) {
  this->onEndCallback = callback;
}

void AViShaOTA::onProgress(void (*callback)(unsigned int progress, unsigned int total)) {
  this->onProgressCallback = callback;
}

void AViShaOTA::onError(void (*callback)(ota_error_t error)) {
  this->onErrorCallback = callback;
}

void AViShaOTA::onWiFiConnected(void (*callback)()) {
  this->onWiFiConnectedCallback = callback;
}

void AViShaOTA::onWiFiDisconnected(void (*callback)()) {
  this->onWiFiDisconnectedCallback = callback;
}

void AViShaOTA::onWebUpdateStart(void (*callback)()) {
  this->onWebUpdateStartCallback = callback;
}

void AViShaOTA::onWebUpdateEnd(void (*callback)(bool success)) {
  this->onWebUpdateEndCallback = callback;
}

void AViShaOTA::end() {
  if (server) {
    server->stop();
  }
  ArduinoOTA.end();
  if (mdnsEnabled) {
    MDNS.end();
  }
  isInitialized = false;
  otaInProgress = false;
  webUpdateInProgress = false;
}

bool AViShaOTA::isOTAInProgress() {
  return otaInProgress;
}

bool AViShaOTA::isWebUpdateInProgress() {
  return webUpdateInProgress;
}

const char* AViShaOTA::getVersion() {
  return AVISHA_OTA_VERSION;
}

// Main begin method
bool AViShaOTA::begin(const char* ssid, const char* password) {
  if (isInitialized) {
    if (serialDebug) {
      Serial.println("AViShaOTA already initialized!");
    }
    return true;
  }

  if (!ssid || strlen(ssid) == 0) {
    if (serialDebug) {
      Serial.println("Error: SSID cannot be empty!");
    }
    return false;
  }

  if (serialDebug) {
    Serial.println("Starting AViShaOTA...");
  }

  // Create server instance
  if (server) {
    delete server;
  }
  server = new WebServer(serverPort);

  // Setup WiFi
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(wifiEventHandler);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, password);

  if (serialDebug) {
    Serial.print("Connecting to WiFi");
  }

  // Wait for connection with timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(500);
    if (serialDebug) {
      Serial.print(".");
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (serialDebug) {
      Serial.println("\nFailed to connect to WiFi!");
    }
    return false;
  }

  if (serialDebug) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }

  // Setup OTA and Web Server
  setupArduinoOTA();
  setupWebServer();

  // Start MDNS if enabled
  if (mdnsEnabled) {
    if (MDNS.begin(hostname.c_str())) {
      if (serialDebug) {
        Serial.println("MDNS responder started");
      }
    } else {
      if (serialDebug) {
        Serial.println("Error starting MDNS responder!");
      }
    }
  }

  // Start services
  ArduinoOTA.begin();
  server->begin();

  if (serialDebug) {
    Serial.println("AViShaOTA started successfully!");
    Serial.print("Upload URL: ");
    Serial.println(getUploadURL());
  }

  isInitialized = true;
  return true;
}

// Handle method - call this in loop()
void AViShaOTA::handle() {
  if (WiFi.status() == WL_CONNECTED && isInitialized) {
    ArduinoOTA.handle();
    if (server) {
      server->handleClient();
    }
  }
}

// Setup ArduinoOTA
void AViShaOTA::setupArduinoOTA() {
  ArduinoOTA.setHostname(hostname.c_str());
  
  if (otaPassword.length() > 0) {
    ArduinoOTA.setPassword(otaPassword.c_str());
  }

  ArduinoOTA.onStart([this]() {
    otaInProgress = true;
    if (serialDebug) {
      Serial.println("OTA Update started...");
    }
    if (onStartCallback) {
      onStartCallback();
    }
  });

  ArduinoOTA.onEnd([this]() {
    otaInProgress = false;
    if (serialDebug) {
      Serial.println("\nOTA Update completed!");
    }
    if (onEndCallback) {
      onEndCallback();
    }
  });

  ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
    if (serialDebug) {
      Serial.printf("OTA Progress: %u%%\r", (progress * 100) / total);
    }
    if (onProgressCallback) {
      onProgressCallback(progress, total);
    }
  });

  ArduinoOTA.onError([this](ota_error_t error) {
    otaInProgress = false;
    if (serialDebug) {
      Serial.printf("OTA Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    }
    if (onErrorCallback) {
      onErrorCallback(error);
    }
  });
}

// Setup Web Server
void AViShaOTA::setupWebServer() {
  server->on("/", HTTP_GET, [this]() {
    handleRoot();
  });

  server->on("/update", HTTP_POST, [this]() {
    handleUpdateFinish();
  }, [this]() {
    handleUpdate();
  });

  server->onNotFound([this]() {
    server->send(404, "text/plain", "Not Found");
  });
}

// Handle root request
void AViShaOTA::handleRoot() {
  server->send(200, "text/html", getUploadHTML());
}

// Handle update finish
void AViShaOTA::handleUpdateFinish() {
  webUpdateInProgress = false;
  if (Update.hasError()) {
    if (serialDebug) {
      Serial.println("Web Update failed!");
    }
    server->send(500, "text/plain", "Update failed");
    if (onWebUpdateEndCallback) {
      onWebUpdateEndCallback(false);
    }
  } else {
    if (serialDebug) {
      Serial.println("Web Update successful!");
    }
    server->send(200, "text/plain", "Update successful! ESP32 will restart...");
    if (onWebUpdateEndCallback) {
      onWebUpdateEndCallback(true);
    }
    delay(1000);
    ESP.restart();
  }
}

// FIXED: Handle update request with proper password validation
void AViShaOTA::handleUpdate() {
  HTTPUpload& upload = server->upload();
  static bool passwordChecked = false;
  static bool passwordValid = false;

  if (upload.status == UPLOAD_FILE_START) {
    // Reset password validation flags
    passwordChecked = false;
    passwordValid = false;
    
    // Check password if set - FIXED: Get password from multipart form
    if (otaPassword.length() > 0) {
      // Try different ways to get the password from multipart form
      String receivedPassword = "";
      
      // Method 1: Try to get from server args (works for some cases)
      if (server->hasArg("password")) {
        receivedPassword = server->arg("password");
      }
      
      // Method 2: Check in multipart form data
      if (receivedPassword.length() == 0) {
        // The password might be in the multipart data before file upload starts
        // We need to store it when it comes through
        for (int i = 0; i < server->args(); i++) {
          if (server->argName(i) == "password") {
            receivedPassword = server->arg(i);
            break;
          }
        }
      }
      
      if (serialDebug) {
        Serial.println("Checking OTA password...");
        Serial.print("Expected: '"); Serial.print(otaPassword); Serial.println("'");
        Serial.print("Received: '"); Serial.print(receivedPassword); Serial.println("'");
      }
      
      if (receivedPassword != otaPassword) {
        if (serialDebug) {
          Serial.println("OTA: Password mismatch - access denied");
        }
        server->send(401, "text/plain", "Unauthorized: Invalid password");
        return;
      }
      
      passwordValid = true;
    } else {
      passwordValid = true; // No password required
    }
    
    passwordChecked = true;
    webUpdateInProgress = true;
    
    if (serialDebug) {
      Serial.printf("Web Update Start: %s\n", upload.filename.c_str());
    }
    
    if (onWebUpdateStartCallback) {
      onWebUpdateStartCallback();
    }

    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      if (serialDebug) {
        Serial.println("Update.begin() failed:");
        Update.printError(Serial);
      }
      webUpdateInProgress = false;
      return;
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    // Only proceed if password was validated
    if (!passwordChecked || !passwordValid) {
      return;
    }
    
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      if (serialDebug) {
        Serial.println("Update.write() failed:");
        Update.printError(Serial);
      }
      webUpdateInProgress = false;
      return;
    }

    if (serialDebug) {
      Serial.printf("Web Update Progress: %d%%\r", (Update.progress() * 100) / Update.size());
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (!passwordChecked || !passwordValid) {
      return;
    }
    
    if (Update.end(true)) {
      if (serialDebug) {
        Serial.printf("\nWeb Update Success: %u bytes\n", upload.totalSize);
      }
    } else {
      if (serialDebug) {
        Serial.println("Update.end() failed:");
        Update.printError(Serial);
      }
      webUpdateInProgress = false;
    }
  }
  else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
    webUpdateInProgress = false;
    if (serialDebug) {
      Serial.println("Web Update was aborted");
    }
  }
}

// Static WiFi event handler
void AViShaOTA::wifiEventHandler(WiFiEvent_t event) {
  if (instance) {
    instance->handleWiFiEvent(event);
  }
}

// WiFi event handler
void AViShaOTA::handleWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_DISCONNECTED:
      if (serialDebug) {
        Serial.println("WiFi disconnected, attempting to reconnect...");
      }
      if (onWiFiDisconnectedCallback) {
        onWiFiDisconnectedCallback();
      }
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      if (serialDebug) {
        Serial.println("WiFi connected!");
      }
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      if (serialDebug) {
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Upload URL: ");
        Serial.println(getUploadURL());
      }
      if (onWiFiConnectedCallback) {
        onWiFiConnectedCallback();
      }
      break;
    default:
      break;
  }
}

// Utility methods
String AViShaOTA::getLocalIP() {
  return WiFi.localIP().toString();
}

String AViShaOTA::getUploadURL() {
  return "http://" + getLocalIP() + ":" + String(serverPort) + "/";
}

bool AViShaOTA::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void AViShaOTA::restart() {
  ESP.restart();
}

// FIXED: Updated HTML with better password handling
const char* AViShaOTA::getUploadHTML() {
  static const char* uploadHTML = R"(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>AViSha OTA Update</title>
  <style>
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen,
        Ubuntu, Cantarell, "Open Sans", "Helvetica Neue", sans-serif;
      margin: 0;
      padding: 0;
      background: #f2f2f7;
    }

    .container {
      max-width: 400px;
      margin: 80px auto;
      background: #fff;
      border-radius: 20px;
      box-shadow: 0 8px 20px rgba(0, 0, 0, 0.08);
      padding: 30px;
      text-align: center;
    }

    h1 {
      font-size: 24px;
      margin-bottom: 10px;
      color: #111;
    }

    p {
      color: #555;
      font-size: 14px;
      margin-bottom: 20px;
    }

    .upload-area {
      border: 2px dashed #d1d1d6;
      border-radius: 12px;
      padding: 30px 10px;
      background-color: #fafafa;
      transition: background 0.3s ease;
    }

    .upload-area:hover {
      background: #f0f0f5;
    }

    input[type="password"], input[type="file"] {
      margin-top: 15px;
      padding: 10px;
      border: 1px solid #d1d1d6;
      border-radius: 8px;
      font-size: 14px;
      width: 80%;
      max-width: 250px;
    }

    input[type="file"] {
      cursor: pointer;
      padding: 8px;
    }

    button {
      margin-top: 20px;
      background-color: #007aff;
      color: white;
      border: none;
      padding: 12px 24px;
      font-size: 16px;
      border-radius: 12px;
      cursor: pointer;
      transition: background 0.3s ease;
      min-width: 150px;
    }

    button:hover:not(:disabled) {
      background-color: #005ed9;
    }

    button:disabled {
      background-color: #8e8e93;
      cursor: not-allowed;
    }

    .progress {
      width: 100%;
      background-color: #e5e5ea;
      border-radius: 12px;
      margin-top: 25px;
      height: 12px;
      overflow: hidden;
      display: none;
    }

    .progress-bar {
      height: 100%;
      width: 0%;
      background-color: #34c759;
      transition: width 0.3s ease;
    }

    #status {
      margin-top: 25px;
      font-size: 14px;
      color: #333;
    }

    .status-success {
      color: #28a745;
    }

    .status-error {
      color: #ff3b30;
    }

    .file-info {
      margin-top: 10px;
      font-size: 12px;
      color: #666;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 OTA Update</h1>
    <p>Select a .bin file to update your device's firmware.</p>

    <div class="upload-area">
      <form id="uploadForm" enctype="multipart/form-data">
        <input type="password" name="password" id="passwordInput" placeholder="Enter OTA Password" />
        <br />
        <input type="file" name="update" id="fileInput" accept=".bin" required />
        <div class="file-info" id="fileInfo"></div>
        <br />
        <button type="submit" id="uploadBtn">Upload and Update</button>
      </form>
    </div>

    <div class="progress" id="progressContainer">
      <div class="progress-bar" id="progressBar"></div>
    </div>

    <div id="status"></div>
  </div>

  <script>
    const uploadForm = document.getElementById("uploadForm");
    const fileInput = document.getElementById("fileInput");
    const passwordInput = document.getElementById("passwordInput");
    const progressContainer = document.getElementById("progressContainer");
    const progressBar = document.getElementById("progressBar");
    const status = document.getElementById("status");
    const uploadBtn = document.getElementById("uploadBtn");
    const fileInfo = document.getElementById("fileInfo");

    fileInput.addEventListener("change", function() {
      const file = this.files[0];
      if (file) {
        const sizeMB = (file.size / (1024 * 1024)).toFixed(2);
        fileInfo.textContent = `File: ${file.name} (${sizeMB} MB)`;
      } else {
        fileInfo.textContent = "";
      }
    });

    uploadForm.addEventListener("submit", function (e) {
      e.preventDefault();

      const file = fileInput.files[0];
      const password = passwordInput.value.trim();

      if (!file || !file.name.endsWith(".bin")) {
        alert("Please select a valid .bin file.");
        return;
      }

      // Create FormData and append fields in correct order
      const formData = new FormData();
      
      // Add password first (if provided)
      if (password) {
        formData.append("password", password);
      }
      
      // Then add the file
      formData.append("update", file);

      const xhr = new XMLHttpRequest();
      
      // Disable upload button
      uploadBtn.disabled = true;
      uploadBtn.textContent = "Uploading...";
      progressContainer.style.display = "block";
      status.innerHTML = "";

      xhr.upload.addEventListener("progress", function (e) {
        if (e.lengthComputable) {
          const percent = Math.round((e.loaded / e.total) * 100);
          progressBar.style.width = percent + "%";
          status.innerHTML = `<p>Uploading: ${percent}%</p>`;
        }
      });

      xhr.addEventListener("load", function () {
        uploadBtn.disabled = false;
        uploadBtn.textContent = "Upload and Update";
        
        if (xhr.status === 200) {
          progressBar.style.width = "100%";
          status.innerHTML = `<p class="status-success">Update successful! Restarting device...</p>`;
          setTimeout(() => {
            status.innerHTML = `<p class="status-success">Restarting... Page will reload.</p>`;
            setTimeout(() => location.reload(), 5000);
          }, 2000);
        } else if (xhr.status === 401) {
          progressContainer.style.display = "none";
          status.innerHTML = `<p class="status-error">Incorrect password! Please check and try again.</p>`;
          passwordInput.focus();
        } else {
          progressContainer.style.display = "none";
          status.innerHTML = `<p class="status-error">Update failed: ${xhr.responseText}</p>`;
        }
      });

      xhr.addEventListener("error", function () {
        uploadBtn.disabled = false;
        uploadBtn.textContent = "Upload and Update";
        progressContainer.style.display = "none";
        status.innerHTML = `<p class="status-error">Network error occurred while uploading.</p>`;
      });

      xhr.addEventListener("timeout", function () {
        uploadBtn.disabled = false;
        uploadBtn.textContent = "Upload and Update";
        progressContainer.style.display = "none";
        status.innerHTML = `<p class="status-error">Upload timeout - please try again.</p>`;
      });

      xhr.timeout = 300000; // 5 minutes timeout
      xhr.open("POST", "/update");
      xhr.send(formData);
    });

    // Focus password field on load if needed
    window.addEventListener('load', function() {
      if (passwordInput.placeholder) {
        passwordInput.focus();
      }
    });
  </script>
</body>
</html>
)";
  return uploadHTML;
}