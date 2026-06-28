/*
 * 警车双闪灯效（双通道反相 PWM）
 * 
 * 功能：
 * - 两个 LED 以反相的占空比交替渐变
 * - 当 LED_A 由暗变亮时，LED_B 由亮变暗，反之亦然
 * - 呈现出类似警车双闪的平滑交替闪烁效果
 * 
 * 硬件连接：
 * - LED_A -> GPIO2（也可使用板载 LED）
 * - LED_B -> GPIO4（需外接 LED + 220Ω 限流电阻）
 */

const int ledPinA = 2;    // LED A 引脚
const int ledPinB = 4;    // LED B 引脚

// PWM 参数
const int freq = 5000;        // 频率 5000 Hz
const int resolution = 8;     // 8 位分辨率 (0 ~ 255)

void setup() {
  Serial.begin(115200);

  // 为两个引脚分别绑定 PWM
  ledcAttach(ledPinA, freq, resolution);
  ledcAttach(ledPinB, freq, resolution);

  Serial.println("警车双闪灯效启动");
}

void loop() {
  // 阶段1：LED_A 逐渐变亮（0→255），LED_B 逐渐变暗（255→0）
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(ledPinA, duty);          // A 逐渐变亮
    ledcWrite(ledPinB, 255 - duty);    // B 逐渐变暗
    delay(8);                          // 控制渐变速度，数值越小变化越快
  }

  // 阶段2：LED_A 逐渐变暗（255→0），LED_B 逐渐变亮（0→255）
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(ledPinA, duty);          // A 逐渐变暗
    ledcWrite(ledPinB, 255 - duty);    // B 逐渐变亮
    delay(8);
  }

  Serial.println("一个完整交替周期完成");
}