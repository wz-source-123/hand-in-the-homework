/*
 * 实时传感器 Web 仪表盘
 * 
 * 功能：
 * - ESP32 作为 Web 服务器，提供一个实时监控页面
 * - 页面自动每秒多次请求触摸传感器模拟值
 * - 显示实时跳动的数值，手靠近时数值下降，远离时恢复
 * 
 * 硬件：
 * - 触摸引脚：GPIO4 (T0)，可外接一段导线或铜箔增加触摸面积
 */

#include <WiFi.h>
#include <WebServer.h>

// WiFi 配置
const char* ssid = "Aemeath";
const char* password = "wangzhong060705";

// 触摸引脚
const int TOUCH_PIN = 4;  // T0

WebServer server(80);

// 生成仪表盘 HTML（包含 AJAX 实时拉取）
String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>触摸传感器仪表盘</title>
  <style>
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      text-align: center;
      background: linear-gradient(135deg, #0f2027, #203a43, #2c5364);
      color: white;
      margin: 0;
      padding: 0;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .dashboard {
      background: rgba(255,255,255,0.1);
      backdrop-filter: blur(10px);
      border-radius: 20px;
      padding: 40px 30px;
      box-shadow: 0 8px 32px rgba(0,0,0,0.3);
      width: 90%;
      max-width: 450px;
    }
    h1 {
      font-size: 2.2rem;
      margin-bottom: 30px;
      text-shadow: 0 2px 4px rgba(0,0,0,0.5);
    }
    .sensor-value {
      font-size: 5rem;
      font-weight: bold;
      background: rgba(0,0,0,0.3);
      padding: 20px;
      border-radius: 15px;
      margin: 20px 0;
      transition: all 0.2s;
      text-shadow: 0 0 10px rgba(255,255,255,0.7);
    }
    .label {
      font-size: 1.2rem;
      opacity: 0.8;
      margin-bottom: 10px;
    }
    .bar {
      width: 100%;
      height: 10px;
      background: rgba(255,255,255,0.2);
      border-radius: 5px;
      margin-top: 25px;
      overflow: hidden;
    }
    .fill {
      height: 100%;
      width: 0%;
      background: #00e676;
      border-radius: 5px;
      transition: width 0.3s;
    }
  </style>
</head>
<body>
  <div class="dashboard">
    <h1>📡 触摸传感器</h1>
    <div class="label">实时数值 (原始 ADC 读数)</div>
    <div id="sensorDisplay" class="sensor-value">--</div>
    <div class="bar">
      <div id="sensorBar" class="fill" style="width: 0%;"></div>
    </div>
    <p style="margin-top:30px; opacity:0.7;">手指靠近 / 触摸引脚 → 数值下降</p>
  </div>

  <script>
    const display = document.getElementById('sensorDisplay');
    const bar = document.getElementById('sensorBar');

    // 定时拉取传感器值（每 200ms）
    setInterval(() => {
      fetch('/value')
        .then(response => {
          if (!response.ok) throw new Error('网络错误');
          return response.text();
        })
        .then(data => {
          let val = parseInt(data.trim());
          if (isNaN(val)) return;
          
          display.textContent = val;
          
          // 将数值映射到进度条（假设 0-100，可根据实际调整上限）
          let maxVal = 100;   // 可根据串口观察到的最值调整
          let percent = Math.min(100, Math.max(0, (val / maxVal) * 100));
          bar.style.width = percent + '%';
          
          // 根据数值大小改变颜色：低数值（触摸）变红
          if (val < 30) {
            display.style.color = '#ff5252';
          } else {
            display.style.color = 'white';
          }
        })
        .catch(err => {
          display.textContent = 'Err';
          console.error(err);
        });
    }, 200);
  </script>
</body>
</html>
)rawliteral";
  return html;
}

// 根路径：返回仪表盘页面
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// 返回纯文本触摸值
void handleValue() {
  int touchValue = touchRead(TOUCH_PIN);
  server.send(200, "text/plain", String(touchValue));
}

void setup() {
  Serial.begin(115200);
  delay(1000);

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
  server.on("/value", handleValue);  // AJAX 轮询此路径获取数据
  server.begin();
  Serial.println("Web 服务器已启动");
}

void loop() {
  server.handleClient();   // 处理客户端请求
}