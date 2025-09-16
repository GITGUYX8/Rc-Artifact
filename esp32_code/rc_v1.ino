#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// === Motor A (Left) ===
#define ENA 5
#define IN1 14
#define IN2 27

// === Motor B (Right) ===
#define ENB 23
#define IN3 25
#define IN4 26

// ====== WiFi credentials ======
const char* ssid = "artifact";
const char* password = "artifact";

// WebSocket only (no HTTP server)
WebSocketsServer webSocket(81);

// ========== Motor Functions ==========
void motor_stop_all() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void move_forward(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void move_backward(int speed) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void left_turn(int speed) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);   // Left motor backward
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // Right motor forward
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void right_turn(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // Left motor forward
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);   // Right motor backward
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

// ========== WebSocket Handler ==========
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[%u] Connected\n", num);
      break;

    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected\n", num);
      break;

    case WStype_TEXT: {
      StaticJsonDocument<128> doc;
      if (deserializeJson(doc, payload)) {
        Serial.println("JSON parse failed");
        return;
      }

      const char* command = doc["command"];
      int pwm = doc["pwm"] | 200;

      if (strcmp(command,"forward")==0) move_forward(pwm);
      else if (strcmp(command,"backward")==0) move_backward(pwm);
      else if (strcmp(command,"left")==0) left_turn(pwm);
      else if (strcmp(command,"right")==0) right_turn(pwm);
      else if (strcmp(command,"stop")==0) motor_stop_all();

      Serial.printf("Command: %s | PWM: %d\n", command, pwm);
    }
    break;
  }
}

// ========== Setup & Loop ==========
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  motor_stop_all();

  // WiFi STA mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to WiFi");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop(); // Real-time control
}
