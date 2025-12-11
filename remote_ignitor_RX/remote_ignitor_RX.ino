HardwareSerial loraSerial(2);

#define RX 16
#define TX 17

#define RELAY_CH1 32
#define RELAY_CH2 33
#define RELAY_CH3 25
#define RELAY_CH4 26

int currentChannel = 1;

void fire() {
  Serial.println("點火");
  if (currentChannel == 1) digitalWrite(RELAY_CH1, HIGH);
  else if (currentChannel == 2) digitalWrite(RELAY_CH2, HIGH);
  
  else if (currentChannel == 3) digitalWrite(RELAY_CH3, HIGH);
  else if (currentChannel == 4) digitalWrite(RELAY_CH4, HIGH);

  delay(5000); // 保持點火時間

  Serial.println("off");
  digitalWrite(RELAY_CH1, LOW);
  digitalWrite(RELAY_CH2, LOW);
  digitalWrite(RELAY_CH3, LOW);
  digitalWrite(RELAY_CH4, LOW);
}

void checkLoRaReceive() {
  if (loraSerial.available()) {
    String loraData = loraSerial.readString();
    Serial.println(loraData);
    if (loraData.indexOf("FIRE") > 0) {
      fire();
    }
    else if (loraData.indexOf("CH1") > 0) {
      currentChannel = 1;
    }
    else if (loraData.indexOf("CH2") > 0) {
      currentChannel = 2;
    }
    else if (loraData.indexOf("CH3") > 0) {
      currentChannel = 3;
    }
    else if (loraData.indexOf("CH4") > 0) {
      currentChannel = 4;
    }
  }
}

void setup() {
  Serial.begin(115200);
  loraSerial.begin(9600, SERIAL_8N1, RX, TX);

  pinMode(RELAY_CH1, OUTPUT);
  pinMode(RELAY_CH2, OUTPUT);
  pinMode(RELAY_CH3, OUTPUT);
  pinMode(RELAY_CH4, OUTPUT);

  digitalWrite(RELAY_CH1, LOW);
  digitalWrite(RELAY_CH2, LOW);
  digitalWrite(RELAY_CH3, LOW);
  digitalWrite(RELAY_CH4, LOW);

  loraSerial.println("AT+OPMODE=1"); 
  delay(500);
  loraSerial.println("AT+ADDRESS=100");
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
  checkLoRaReceive();
}
