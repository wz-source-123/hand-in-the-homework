/*
 * 物联网安防报警器模拟（修正变量名）
 * 
 * 功能：
 * - 网页提供“布防(Arm)”和“撤防(Disarm)”按钮
 * - 布防状态下，触摸 T0 引脚立即触发报警，LED 高频闪烁
 * - 报警锁定，即使松开触摸仍持续闪烁
 * - 只有点击“撤防”才能关闭报警并熄灭 LED
 * - 未布防时触摸无效
 * 
 * 硬件：
 * - 触摸引脚: GPIO4 (T0)
 * - LED 引脚: GPIO2 (板载 LED)
 */

#include <WiFi.h>
#include <WebServer.h>

// WiFi 配置
const char* ssid = "Aemeath";
const char* password = "wangzhong060705";

// 引脚定义
const int TOUCH_PIN = 4;
const int LED_PIN = 2;

// 触摸阈值（需根据实际硬件调整）
const int THRESHOLD = 400;
// 防抖延时（毫秒）
const unsigned long DEBOUNCE_DELAY = 50;
// LED 闪烁间隔（毫秒），值越小闪烁越快
const unsigned long BLINK_INTERVAL = 150;

WebServer server(80);

// 系统状态（注意：变量名不要与标准库函数 alarm 冲突）
bool armed = false;          // 布防状态
bool alarmActive = false;    // 报警状态（已触发）
bool ledState = false;       // LED 当前电平（用于闪烁）

// 触摸防抖相关变量
bool lastRawTouch = false;
bool stableTouch = false;
bool lastStableTouch = false;
unsigned long lastDebounceTime = 0;

// 非阻塞 LED 闪烁计时
unsigned long lastBlinkTime = 0;

// 生成网页 HTML
String makePage() {
  String statusText;
  String armButton, disarmButton;

  if (alarmActive) {
    statusText = "⚠️ 报警中！请立即撤防！";
    armButton = "";
    disarmButton = "<a href=\"/disarm\"><button style=\"padding:12px 24px; font-size:18px; background:red; color:white;\">撤防 (Disarm)</button></a>";
  } else if (armed) {
    statusText = "✅ 已布防，触摸检测中...";
    armButton = "";
    disarmButton = "<a href=\"/disarm\"><button style=\"padding:12px 24px; font-size:18px;\">撤防 (Disarm)</button></a>";
  } else {
    statusText = "🔓 未布防，系统空闲";
    armButton = "<a href=\"/arm\"><button style=\"padding:12px 24px; font-size:18px; background:green; color:white;\">布防 (Arm)</button></a>";
    disarmButton = "";
  }

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>安防报警器</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 80px; background: #1e1e1e; color: white; }
    .container { background: #333; padding: 30px; margin: auto; width: 80%; max-width: 400px; border-radius: 10px; }
    button { margin: 10px; border: none; border-radius: 5px; cursor: pointer; }
  </style>
</head>
<body>
  <div class="container">
    <h1>安防报警器</h1>
    <p style="font-size:20px;">状态：<b>)rawliteral" + statusText + R"rawliteral(</b></p>
    )rawliteral" + armButton + disarmButton + R"rawliteral(
  </div>
</body>
</html>
)rawliteral";
  return html;
}

// 根路径处理
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// 布防
void handleArm() {
  armed = true;
  alarmActive = false;      // 布防同时清除报警
  digitalWrite(LED_PIN, LOW);
  Serial.println("系统布防");
  server.sendHeader("Location", "/");
  server.send(303);
}

// 撤防
void handleDisarm() {
  armed = false;
  alarmActive = false;      // 撤防关闭报警
  digitalWrite(LED_PIN, LOW);
  Serial.println("系统撤防");
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 连接 WiFi
  Serial.print("连接 WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi 已连接");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  // 注册 Web 路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();
  Serial.println("服务器已启动");
}

void loop() {
  server.handleClient();   // 处理 Web 请求

  // 1. 触摸检测（含防抖）
  int touchValue = touchRead(TOUCH_PIN);
  bool currentRawTouch = (touchValue < THRESHOLD);

  if (currentRawTouch != lastRawTouch) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (currentRawTouch != stableTouch) {
      stableTouch = currentRawTouch;
    }
  }

  // 检测触摸上升沿（从未触摸到触摸）
  if (lastStableTouch == false && stableTouch == true) {
    if (armed && !alarmActive) {   // 仅布防且未报警时才触发
      alarmActive = true;
      Serial.println("!!! 报警触发 !!!");
    }
  }

  lastStableTouch = stableTouch;
  lastRawTouch = currentRawTouch;

  // 2. 报警状态下的 LED 狂闪
  if (alarmActive) {
    if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  } else {
    // 非报警状态下确保 LED 熄灭
    digitalWrite(LED_PIN, LOW);
  }
}