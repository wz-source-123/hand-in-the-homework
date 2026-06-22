// 定义LED引脚，ESP32通常板载LED连接在GPIO 2
const int ledPin = 2;

// ---------- SOS 序列定义 ----------
// 每步包含：LED状态(HIGH/LOW) 和 保持时间(毫秒)
struct Step {
  bool state;
  unsigned int duration;
};

const Step sosSequence[] = {
  // S: 短闪3次 (亮200ms, 灭200ms)
  {HIGH, 200}, {LOW, 200},
  {HIGH, 200}, {LOW, 200},
  {HIGH, 200}, {LOW, 200},
  // 字母间隔 (灭500ms)
  {LOW, 500},
  // O: 长闪3次 (亮600ms, 灭200ms)
  {HIGH, 600}, {LOW, 200},
  {HIGH, 600}, {LOW, 200},
  {HIGH, 600}, {LOW, 200},
  // 字母间隔 (灭500ms)
  {LOW, 500},
  // S: 短闪3次
  {HIGH, 200}, {LOW, 200},
  {HIGH, 200}, {LOW, 200},
  {HIGH, 200}, {LOW, 200},
  // 单词间隔 (长灭2000ms)
  {LOW, 2000}
};

const int sequenceLength = sizeof(sosSequence) / sizeof(Step);

// ---------- 状态变量 ----------
int currentStep = 0;                  // 当前步骤索引
unsigned long stepStartTime = 0;      // 当前步骤开始的时间

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // 初始化为序列第一个状态（亮200ms）
  digitalWrite(ledPin, sosSequence[0].state);
  Serial.println("LED ON");           // 对应初始状态
  stepStartTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // 检查是否到达当前步骤的结束时间
  if (currentMillis - stepStartTime >= sosSequence[currentStep].duration) {
    // 进入下一步（循环）
    stepStartTime = currentMillis;
    currentStep = (currentStep + 1) % sequenceLength;

    // 应用新步骤的LED状态
    bool newState = sosSequence[currentStep].state;
    digitalWrite(ledPin, newState);

    // 输出提示（每次切换状态时输出）
    if (newState == HIGH) {
      Serial.println("LED ON");
    } else {
      Serial.println("LED OFF");
    }
  }

  // 此处可以添加其他非阻塞任务，不会影响SOS闪烁
}