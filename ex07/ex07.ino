/*
 * Web 无极调光器
 * 
 * 功能：
 * - 手机或电脑连接 ESP32 热点或同一局域网
 * - 浏览器打开 ESP32 IP 地址
 * - 拖动滑动条，LED 亮度实时跟随变化
 * 
 * 硬件：
 * - LED 引脚: GPIO2 (板载 LED)
 */

#include <WiFi.h>
#include <WebServer.h>

// WiFi 配置
const char* ssid = "Aemeath";
const char* password = "wangzhong060705";

// 硬件定义
const int ledPin = 2;             // LED PWM 引脚
const int freq = 5000;            // PWM 频率
const int resolution = 8;        // 8 位分辨率 (0-255)

WebServer server(80);            // Web 服务器监听 80 端口

// 初始亮度（范围 0-255）
int brightness = 0;

// 生成 HTML 页面，包含滑动条与 JavaScript
String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>无极调光器</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin-top: 80px;
      background-color: #f4f4f4;
    }
    .container {
      background: white;
      padding: 30px;
      margin: 0 auto;
      width: 80%;
      max-width: 400px;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.2);
    }
    input[type=range] {
      width: 100%;
      margin: 20px 0;
    }
    .value {
      font-size: 24px;
      font-weight: bold;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>LED 无极调光</h1>
    <input type="range" id="brightnessSlider" min="0" max="255" value="0" step="1">
    <p>当前亮度: <span id="brightnessValue" class="value">0</span></p>
  </div>

  <script>
    const slider = document.getElementById('brightnessSlider');
    const valueDisplay = document.getElementById('brightnessValue');

    // 页面加载后从服务器获取当前亮度值并设置滑块位置
    fetch('/get')
      .then(response => response.text())
      .then(data => {
        slider.value = data;
        valueDisplay.textContent = data;
      })
      .catch(err => console.error('获取初始亮度失败:', err));

    // 监听滑块变化，实时发送请求
    slider.addEventListener('input', function() {
      let val = this.value;
      valueDisplay.textContent = val;
      // 通过 GET 请求将亮度值发送给 ESP32
      fetch('/set?value=' + val)
        .catch(err => console.error('请求失败:', err));
    });
  </script>
</body>
</html>
)rawliteral";
  return html;
}

// 处理根路径，返回网页
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// 处理设置亮度请求：GET /set?value=xxx
void handleSet() {
  if (server.hasArg("value")) {
    String valueStr = server.arg("value");
    int val = valueStr.toInt();
    // 限制有效范围
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    brightness = val;
    ledcWrite(ledPin, brightness);
    server.send(200, "text/plain", "OK");
    Serial.print("亮度设置为: ");
    Serial.println(brightness);
  } else {
    server.send(400, "text/plain", "Missing value");
  }
}

// 处理获取当前亮度请求：GET /get
void handleGet() {
  server.send(200, "text/plain", String(brightness));
}

void setup() {
  Serial.begin(115200);
  
  // 初始化 PWM
  ledcAttach(ledPin, freq, resolution);
  ledcWrite(ledPin, brightness);
  
  // 连接 WiFi
  Serial.print("正在连接 WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi 已连接");
  Serial.print("请在浏览器中打开: http://");
  Serial.println(WiFi.localIP());
  
  // 注册路由
  server.on("/", handleRoot);
  server.on("/set", HTTP_GET, handleSet);   // 处理设置请求
  server.on("/get", HTTP_GET, handleGet);   // 处理获取请求
  server.begin();
  Serial.println("Web 服务器启动");
}

void loop() {
  server.handleClient();   // 持续处理客户端请求
}
