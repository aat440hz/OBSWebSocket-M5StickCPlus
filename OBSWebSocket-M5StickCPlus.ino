#include <M5StickCPlus.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <mbedtls/md.h>
#include <mbedtls/base64.h>

using namespace websockets;

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* obsHost = "OBS_IP_ADDRESS";
const int obsPort = 4455;
const char* obsPassword = "YOUR_OBS_WEBSOCKET_PASSWORD";

WebsocketsClient webSocket;
String currentSceneName = "Unknown";
bool isStreaming = false;
bool obsConnected = false;
unsigned long lastStatusUpdate = 0;
const unsigned long STATUS_INTERVAL = 5000;

void updateDisplay() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  
  // Using larger text, we might need to adjust layout
  M5.Lcd.printf("WiFi:%s OBS:%s\n", 
    WiFi.status() == WL_CONNECTED ? "OK" : "NO",
    obsConnected ? "OK" : "NO");
  M5.Lcd.setCursor(0, 20);  // Move down for next line
  M5.Lcd.printf("Stream:%s\n", isStreaming ? "ON" : "OFF");
  M5.Lcd.setCursor(0, 40);  // Move down again
  M5.Lcd.printf("Scene:%s\n", currentSceneName.c_str());
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void handleHelloMessage(const JsonDocument& doc) {
  obsConnected = true;
  updateDisplay();

  if (doc["d"].containsKey("authentication")) {
    const char* challenge = doc["d"]["authentication"]["challenge"].as<const char*>();
    const char* salt = doc["d"]["authentication"]["salt"].as<const char*>();

    unsigned char password_salt_hash[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char*)(String(obsPassword) + String(salt)).c_str(), strlen((String(obsPassword) + String(salt)).c_str()));
    mbedtls_md_finish(&ctx, password_salt_hash);
    mbedtls_md_free(&ctx);

    char password_salt_hash_base64[45];
    size_t olen;
    mbedtls_base64_encode((unsigned char*)password_salt_hash_base64, 45, &olen, password_salt_hash, 32);

    unsigned char final_hash[32];
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char*)(String(password_salt_hash_base64) + String(challenge)).c_str(), strlen((String(password_salt_hash_base64) + String(challenge)).c_str()));
    mbedtls_md_finish(&ctx, final_hash);
    mbedtls_md_free(&ctx);

    char final_auth_base64[45];
    mbedtls_base64_encode((unsigned char*)final_auth_base64, 45, &olen, final_hash, 32);

    StaticJsonDocument<200> authDoc;
    authDoc["op"] = 1;
    authDoc["d"]["rpcVersion"] = 1;
    authDoc["d"]["authentication"] = final_auth_base64;
    authDoc["d"]["eventSubscriptions"] = 65535;

    String output;
    serializeJson(authDoc, output);
    webSocket.send(output);
    
    fetchStreamStatus();
    fetchCurrentScene();
  }
}

void sendCommand(const char* requestType, const JsonDocument& payload) {
  StaticJsonDocument<256> doc;
  doc["op"] = 6;
  doc["d"]["requestType"] = requestType;
  doc["d"]["requestId"] = "1";
  doc["d"]["requestData"] = payload;

  String output;
  serializeJson(doc, output);
  webSocket.send(output);
}

void fetchCurrentScene() {
  DynamicJsonDocument data(1024);
  sendCommand("GetCurrentProgramScene", data);
}

void fetchStreamStatus() {
  DynamicJsonDocument data(1024);
  sendCommand("GetStreamStatus", data);
}

void switchScene() {
  DynamicJsonDocument data(1024);
  sendCommand("GetSceneList", data);
}

void toggleStreaming() {
  DynamicJsonDocument data(1024);
  sendCommand("ToggleStream", data);
  delay(1000);
  fetchStreamStatus();
}

void onMessageCallback(WebsocketsMessage message) {
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, message.data());
  if (error) {
    Serial.println("JSON Parse Error");
    return;
  }

  int op = doc["op"].as<int>();
  if (op == 0) {
    handleHelloMessage(doc);
  } else if (op == 7) {
    String requestType = doc["d"]["requestType"].as<String>();

    if (requestType == "GetSceneList") {
      JsonArray scenes = doc["d"]["responseData"]["scenes"].as<JsonArray>();
      int sceneCount = scenes.size();
      if (sceneCount == 0) return;

      int currentIndex = -1;
      for (int i = 0; i < sceneCount; i++) {
        if (scenes[i]["sceneName"] == currentSceneName) {
          currentIndex = i;
          break;
        }
      }
      
      int nextIndex = (currentIndex + 1) % sceneCount;
      String nextScene = scenes[nextIndex]["sceneName"].as<String>();
      
      DynamicJsonDocument data(1024);
      data["sceneName"] = nextScene;
      sendCommand("SetCurrentProgramScene", data);
    }
    else if (requestType == "GetStreamStatus") {
      isStreaming = doc["d"]["responseData"]["outputActive"].as<bool>();
      updateDisplay();
    }
    else if (requestType == "GetCurrentProgramScene") {
      currentSceneName = doc["d"]["responseData"]["currentProgramSceneName"].as<String>();
      updateDisplay();
    }
    else if (requestType == "SetCurrentProgramScene") {
      currentSceneName = doc["d"]["requestData"]["sceneName"].as<String>();
      updateDisplay();
    }
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(2);  // Increase text size (default is 1, 2 doubles it)
  connectToWiFi();
  updateDisplay();

  webSocket.onMessage(onMessageCallback);
  webSocket.connect("ws://" + String(obsHost) + ":" + String(obsPort));
}

void loop() {
  webSocket.poll();
  M5.update();

  if (obsConnected && millis() - lastStatusUpdate >= STATUS_INTERVAL) {
    fetchStreamStatus();
    fetchCurrentScene();
    lastStatusUpdate = millis();
  }

  if (M5.BtnA.wasPressed()) {
    toggleStreaming();
  }
  if (M5.BtnB.wasPressed()) {
    switchScene();
  }
}
