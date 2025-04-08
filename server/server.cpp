#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include <SPI.h>
#include <ESPmDNS.h>

// CONFIGURE HERE
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
#define SD_CS_PIN 5 // Chip select pin for SD card (commonly D5)

// Create AsyncWebServer on port 80
AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not Found");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start mDNS
  if (!MDNS.begin("exec")) {
    Serial.println("Error setting up MDNS!");
    return;
  }
  Serial.println("mDNS responder started: http://exec.local");

  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card Mount Failed");
    return;
  }
  Serial.println("SD Card mounted successfully");

  // Serve files from SD card (root is /www)
  server.serveStatic("/", SD, "/www/").setDefaultFile("index.html");

  // 404 handler
  server.onNotFound(notFound);

  // Start server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Nothing needed here for AsyncWebServer
}
