// 定义LED引脚，ESP32通常板载LED连接在GPIO 2
const int ledPin = 2; 

// 用于非阻塞计时的变量
unsigned long previousMillis = 0;   // 上次切换LED状态的时间
const long interval = 500;          // 切换间隔 500 毫秒（1 Hz 闪烁，亮/灭各半周期）
bool ledState = LOW;                // 当前LED状态（初始熄灭）

void setup() {
  // 初始化串口通信，设置波特率为115200
  Serial.begin(115200);
  // 将LED引脚设置为输出模式
  pinMode(ledPin, OUTPUT);
  // 确保LED初始为熄灭状态
  digitalWrite(ledPin, ledState);
  Serial.println("LED OFF");
}

void loop() {
  // 获取当前系统运行时间（毫秒）
  unsigned long currentMillis = millis();

  // 检查是否达到切换时间点
  if (currentMillis - previousMillis >= interval) {
    // 更新时间基准
    previousMillis = currentMillis;

    // 翻转LED状态
    ledState = !ledState;
    digitalWrite(ledPin, ledState);

    // 根据新状态输出提示
    if (ledState == HIGH) {
      Serial.println("LED ON");
    } else {
      Serial.println("LED OFF");
    }
  }

  // 这里可以放置其他任务，不会被LED闪烁阻塞
}
