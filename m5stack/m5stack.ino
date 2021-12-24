#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "env.h"

struct Response {
  boolean hasError;
  String body;
};

WiFiMulti wifiMulti;
String tasklistUrl = "";
boolean hasError = false;

void setup() {
  M5.begin();
  M5.Power.begin();

  tasklistUrl.concat(APP_URL);
  tasklistUrl.concat("?taskId=");
  tasklistUrl.concat(TASK_ID);

  clearLcd();

  M5.Lcd.print("connectiong wifi");
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
}

void loop() {
  hasError = false;

  showTask();

  if (hasError) {
    delay(10000);
  }
}

void errorLog() {
  if (!hasError) {
    clearLcd();
  }
  hasError = true;
}

void clearLcd() {
  // M5.Lcd.setTextSize(1);
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 0);
}

void showTask() {
  if (wifiMulti.run() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(1000);
    return;
  }

  String tasks[MAX_TASK_NUM] = {};
  boolean ok = loadTasks(tasks);
  if (!ok) {
    return;
  }

  M5.Lcd.loadFont(FONT_FILE, SD);
  clearLcd();
  for (int i = 0; i < MAX_TASK_NUM; i++) {
    if (tasks[i].isEmpty()) {
      break;
    }
    M5.Lcd.printf("- %s\n", tasks[i].c_str());
  }
  M5.Lcd.unloadFont();

  delay(600000);
}

boolean loadTasks(String tasks[MAX_TASK_NUM]) {
  Response res = requestTasks();
  if (res.hasError) {
    return false;
  }

  boolean ok = parseTask(tasks, res.body);
  if (!ok) {
    return false;
  }

  return true;
}

Response requestTasks() {
  Response res = { true, "" };

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.begin(tasklistUrl);

  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    errorLog();
    M5.Lcd.printf("[HTTP] failed to request tasks: %d\n", httpCode);
    return res;
  }

  String body = http.getString();
  res.hasError = false;
  res.body = body;
  return res;
}

boolean parseTask(String tasks[MAX_TASK_NUM], String body) {
  StaticJsonDocument<2048> doc;

  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    errorLog();
    M5.Lcd.printf("[JSON] failed to deserialize tasks: %s\n", error.c_str());
    return false;
  }

  int i = 0;
  for (JsonObject item : doc.as<JsonArray>()) {
    const char* title = item["title"];
    tasks[i++] = String(title);
  }

  return true;
}
