// 引脚定义
const int touchPin = 4;   // 触摸引脚 T0
const int ledPin = 2;     // LED引脚（ESP32板载LED）

// 触摸阈值：低于该值视为“触摸”，需根据实际测试调整
const int threshold = 20;

// 防抖时间（毫秒）：状态必须稳定此时间后才认定为有效
const unsigned long debounceDelay = 50;

// 状态变量
bool ledState = false;              // LED当前状态
bool lastTouchState = false;        // 上一次读取到的触摸状态（true=触摸，false=未触摸）
bool currentTouchState = false;     // 当前读取到的原始触摸状态
bool stableTouchState = false;      // 经防抖稳定后的触摸状态
bool lastStableTouchState = false;  // 上一次稳定后的触摸状态（用于边缘检测）

unsigned long lastDebounceTime = 0; // 上次状态变化的时间戳

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState); // 初始熄灭LED

  // 可选：打印提示
  Serial.println("触摸自锁开关已启动");
  Serial.print("触摸阈值: ");
  Serial.println(threshold);
}

void loop() {
  // 1. 读取原始触摸值并转换为布尔状态
  int touchValue = touchRead(touchPin);
  currentTouchState = (touchValue < threshold);

  // 2. 软件防抖：如果状态发生变化，重置计时器
  if (currentTouchState != lastTouchState) {
    lastDebounceTime = millis(); // 记录变化时刻
  }

  // 3. 当状态保持稳定超过防抖时间，才更新为稳定状态
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 如果稳定状态与之前不同，则真正更新
    if (currentTouchState != stableTouchState) {
      stableTouchState = currentTouchState;
    }
  }

  // 4. 边缘检测：检测从“未触摸”到“触摸”的上升沿
  if (lastStableTouchState == false && stableTouchState == true) {
    // 检测到一次有效的触摸动作
    ledState = !ledState;                      // 翻转LED状态
    digitalWrite(ledPin, ledState);            // 更新LED
    Serial.print("触摸检测到，LED状态切换为: ");
    Serial.println(ledState ? "ON" : "OFF");
  }

  // 5. 更新历史状态，为下一次循环做准备
  lastStableTouchState = stableTouchState;
  lastTouchState = currentTouchState;

  // 短延时，降低CPU占用，同时保证响应速度
  delay(10);
}