/*
 * 多档位触摸调速呼吸灯
 * 
 * 功能说明：
 * - LED（GPIO2）持续以呼吸灯方式亮灭
 * - 触摸 T0（GPIO4）一次，速度档位循环切换（1 -> 2 -> 3 -> 1 ...）
 * - 三个档位对应不同的呼吸速度（通过改变 PWM 更新间隔实现）
 * - 包含软件防抖，滤除触摸时的抖动
 */

// 引脚定义
const int touchPin = 4;   // 触摸引脚 T0
const int ledPin = 2;     // LED 引脚（板载 LED）

// PWM 参数
const int freq = 5000;
const int resolution = 8;     // 8 位分辨率，占空比范围 0~255

// 触摸阈值：触摸时读数低于此值，需根据实际测试调整
const int threshold = 400;

// 防抖时间（毫秒）
const unsigned long debounceDelay = 50;

// 速度档位对应的呼吸间隔（毫秒），数值越小呼吸越快
const int speedIntervals[3] = {15, 7, 3};  // 档位 1,2,3

// 触摸状态变量（用于边缘检测与防抖）
bool lastRawTouch = false;           // 上一次原始触摸状态
bool currentRawTouch = false;        // 当前原始触摸状态
bool stableTouch = false;            // 防抖后稳定状态
bool lastStableTouch = false;        // 上一次稳定状态（用于检测上升沿）
unsigned long lastDebounceTime = 0;  // 防抖计时

// 呼吸灯状态变量
int dutyCycle = 0;                // 当前占空比
int direction = 1;                // 亮度变化方向：1=增亮, -1=变暗
int currentSpeedLevel = 1;        // 当前速度档位（1,2,3）
unsigned long previousMillis = 0; // 上次 PWM 更新的时间

void setup() {
  Serial.begin(115200);
  
  // 初始化 LED PWM
  ledcAttach(ledPin, freq, resolution);
  ledcWrite(ledPin, dutyCycle);
  
  Serial.println("多档位触摸调速呼吸灯启动");
  Serial.print("当前档位: "); Serial.println(currentSpeedLevel);
}

void loop() {
  // 1. 读取触摸并执行防抖与边缘检测
  int touchValue = touchRead(touchPin);
  currentRawTouch = (touchValue < threshold);
  
  // 如果原始状态改变，重置防抖计时
  if (currentRawTouch != lastRawTouch) {
    lastDebounceTime = millis();
  }
  
  // 防抖：稳定超过 debounceDelay 才更新稳定状态
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentRawTouch != stableTouch) {
      stableTouch = currentRawTouch;
    }
  }
  
  // 边缘检测：从未触摸 -> 触摸（上升沿），切换档位
  if (lastStableTouch == false && stableTouch == true) {
    // 档位循环切换：1 -> 2 -> 3 -> 1 ...
    if (currentSpeedLevel >= 3) {
      currentSpeedLevel = 1;
    } else {
      currentSpeedLevel++;
    }
    Serial.print("触摸检测，切换到档位: ");
    Serial.println(currentSpeedLevel);
  }
  
  // 更新历史状态
  lastStableTouch = stableTouch;
  lastRawTouch = currentRawTouch;
  
  // 2. 非阻塞呼吸灯控制
  int interval = speedIntervals[currentSpeedLevel - 1];  // 当前档位的时间间隔
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    
    // 更新占空比
    dutyCycle += direction;
    
    // 到达边界时反转方向
    if (dutyCycle >= 255) {
      dutyCycle = 255;
      direction = -1;  // 开始变暗
    } else if (dutyCycle <= 0) {
      dutyCycle = 0;
      direction = 1;   // 开始变亮
    }
    
    ledcWrite(ledPin, dutyCycle);
  }
  
  // 极短的延时让出 CPU，同时保持对触摸的快速响应
  delay(1);
}