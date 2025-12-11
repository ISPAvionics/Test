#include <Wire.h>
#include <U8g2lib.h>

HardwareSerial loraSerial(2); // LoRa 模組使用 Serial Port 2
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define RX 16 // LoRa Rx
#define TX 17 // LoRa Tx

#define BTN_CH 5   // 切換通道按鈕
#define BTN_FIRE 3  // 點火按鈕
#define SW_SAFE 4   // 保險開關 (ON=允許點火, OFF=禁止點火)

#define BUZZER_PIN 27 // 蜂鳴器

int currentChannel = 1; // 當前通道

//按鈕去彈跳
unsigned long lastDebounceTime_CH = 0;   // 通道按鈕的計時器
unsigned long lastDebounceTime_FIRE = 0; // 點火按鈕的計時器
unsigned long debounceDelay = 50;        // 50毫秒的去彈跳延遲

//儲存按鈕的穩定
bool lastStableState_CH = HIGH;
bool lastStableState_FIRE = HIGH;

//儲存按鈕的當前原始讀取值
bool reading_CH;
bool reading_FIRE;

//儲存按鈕的上一次原始讀取值
bool lastReading_CH = HIGH;
bool lastReading_FIRE = HIGH;

void fire() {
  //點火前響五次
  for (int i = 5; i > 0; i--) {
    tone(BUZZER_PIN, 1000, 200); 
    delay(800); 
  }

  sendMessage("FIRE");

  //點火後持續響
  tone(BUZZER_PIN, 1000);
  delay(5000);
  noTone(BUZZER_PIN);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr); 
  u8g2.drawStr(0, 60, "Fired");
  u8g2.sendBuffer();
}

void sendMessage(String msg) {
  String sendCmd = "AT+SEND=100," + String(msg.length()) + "," + msg;
  loraSerial.println(sendCmd);
  Serial.println("LoRa TX: " + sendCmd);
}

void setup() {
  Serial.begin(115200); 
  u8g2.begin(); 
  loraSerial.begin(9600, SERIAL_8N1, RX, TX); 

  pinMode(BTN_CH, INPUT_PULLUP);
  pinMode(BTN_FIRE, INPUT_PULLUP);
  pinMode(SW_SAFE, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

  Serial.println("Starting LoRa Configuration...");

// --- LoRa 模組 AT 指令設定 ---
  loraSerial.println("AT+OPMODE=1"); 
  delay(500);
  loraSerial.println("AT+ADDRESS=1");
  delay(500);
  loraSerial.println("AT+BAND=923000000");
  delay(500);
  loraSerial.println("AT+PARAMETER=7,9,4,12");
  delay(500);

  // --- 檢查 LoRa 設定 ---
  loraSerial.println("AT+BAND?");
  delay(100);
  if (loraSerial.available()) {
    String resp = loraSerial.readString();
    Serial.print("Band Response: ");
    Serial.println(resp);
  }
  delay(1000);

  loraSerial.println("AT+ADDRESS?");
  delay(100);
  if (loraSerial.available()) {
    String resp = loraSerial.readString();
    Serial.print("Address Response: ");
    Serial.println(resp);
  }
  delay(1000);
  
  Serial.println("Setup Complete. Running...");
}

void loop() {
  // --- 讀取所有按鈕的「當前」原始狀態 ---
  reading_CH = digitalRead(BTN_CH);
  reading_FIRE = digitalRead(BTN_FIRE);
  bool safeState = digitalRead(SW_SAFE); // 安全開關(撥動開關)通常不需要去彈跳

  // 1. 檢查「原始讀數」是否改變 (這只會在剛按下或剛放開時觸發)
  if (reading_CH != lastReading_CH) {
    // 如果改變了 (無論是彈跳或真的按/放)，重置計時器
    lastDebounceTime_CH = millis();
  }

  // 2. 檢查狀態是否「穩定」超過 debounceDelay
  if ((millis() - lastDebounceTime_CH) > debounceDelay) {
    // 狀態已經穩定 50ms 了
    
    // 檢查這個「穩定的狀態」是否與「上次確認的穩定狀態」不同
    if (reading_CH != lastStableState_CH) {
      lastStableState_CH = reading_CH; 

      if (lastStableState_CH == LOW) {
        // 執行切換通道動作
        currentChannel++;
        if (currentChannel > 4) currentChannel = 1;
        String msg = "CH" + String(currentChannel);
        sendMessage(msg);
        Serial.println("Channel Changed: " + String(currentChannel));
      }
    }
  }

  lastReading_CH = reading_CH;

  // 1. 檢查「原始讀數」是否改變
  if (reading_FIRE != lastReading_FIRE) {
    lastDebounceTime_FIRE = millis();
  }

  // 2. 檢查狀態是否「穩定」
  if ((millis() - lastDebounceTime_FIRE) > debounceDelay) {
    // 狀態穩定
    if (reading_FIRE != lastStableState_FIRE) {
      lastStableState_FIRE = reading_FIRE;
      if (lastStableState_FIRE == LOW) {
        // 檢查安全開關
        if (safeState == LOW) { // 狀態為 ARMED
          Serial.println("FIRE button pressed & ARMED. Firing!");
          fire();
        } else {
          Serial.println("FIRE button pressed, but system is SAFE.");
        }
      }
    }
  }
  
  lastReading_FIRE = reading_FIRE;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 12, "Local Controller");

  char chBuffer[20];
  sprintf(chBuffer, "Channel: %d", currentChannel);
  u8g2.drawStr(0, 28, chBuffer);

  u8g2.drawStr(0, 44, safeState ? "State: SAFE" : "State: ARMED");
  u8g2.sendBuffer();
}