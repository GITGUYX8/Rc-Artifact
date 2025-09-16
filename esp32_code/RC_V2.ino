#include <WiFi.h>
#include <ESPmDNS.h>
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

// WiFi credentials
const char* ssid = "ESP32_ROBOT";
const char* password = "";

// Web servers
WiFiServer server(80);
WebSocketsServer webSocket(81);

// HTML UI (same as before)
const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 L298N Robot</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; }
    button { padding: 15px 25px; margin: 8px; font-size: 16px; border-radius: 5px; border: none; cursor: pointer; }
    .stop { background: #e74c3c; color: white; }
    .forward { background: #2ecc71; color: white; }
    .backward { background: #3498db; color: white; }
    .left { background: #f39c12; color: white; }
    .right { background: #9b59b6; color: white; }
  </style>
</head>
<body>
  <h1>ESP32 Robot Controller</h1>
  <div id="status">Connecting...</div>

  <button class="forward" onclick="sendCommand('robot','forward')">FORWARD (W)</button><br>
  <button class="backward" onclick="sendCommand('robot','backward')">BACKWARD (S)</button><br>
  <button class="left" onclick="sendCommand('robot','left')">LEFT (A)</button>
  <button class="right" onclick="sendCommand('robot','right')">RIGHT (D)</button><br>
  <button class="stop" onclick="sendCommand('robot','stop')">STOP (SPACE)</button>

  <div>
    <p>Speed: <span id="pwmValueRobot">150</span></p>
    <input type="range" min="0" max="255" value="150" id="pwmSliderRobot" oninput="updatePWM(this.value)">
  </div>

<script>
let ws;
let pwm = 150;

function connectWS() {
  ws = new WebSocket('ws://' + location.hostname + ':81/');
  ws.onopen = () => document.getElementById("status").textContent = "Connected";
  ws.onclose = () => { document.getElementById("status").textContent = "Disconnected"; setTimeout(connectWS, 2000); };
  ws.onmessage = (e) => console.log("Received:", e.data);
}

function sendCommand(motor, cmd) {
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ motor: motor, command: cmd, pwm: pwm }));
  }
}

function updatePWM(value) {
  pwm = parseInt(value);
  document.getElementById("pwmValueRobot").textContent = value;
}

document.addEventListener("keydown", e => {
  switch(e.key.toLowerCase()) {
    case "w": sendCommand("robot","forward"); break;
    case "s": sendCommand("robot","backward"); break;
    case "a": sendCommand("robot","left"); break;
    case "d": sendCommand("robot","right"); break;
    case " ": sendCommand("robot","stop"); break;
  }
});

window.onload = connectWS;
</script>
</body>
</html>
)=====";

// ========== Motor Functions (L298N) ==========

void motor_stop_all() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
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

// ========== WebSocket Event ==========

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.printf("[%u] Connected\n", num);
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected\n", num);
      break;
    case WStype_TEXT: {
      StaticJsonDocument<200> doc;
      if (deserializeJson(doc, payload)) return;

      const char* motor = doc["motor"];
      const char* command = doc["command"];
      int pwm = doc["pwm"] | 150;

      if (strcmp(motor,"robot") == 0) {
        if (strcmp(command,"forward")==0) move_forward(pwm);
        else if (strcmp(command,"backward")==0) move_backward(pwm);
        else if (strcmp(command,"left")==0) left_turn(pwm);
        else if (strcmp(command,"right")==0) right_turn(pwm);
        else if (strcmp(command,"stop")==0) motor_stop_all();
      }
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

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_ROBOT", "");
  Serial.println("WiFi AP Started: ESP32_ROBOT");
  Serial.println(WiFi.softAPIP());

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    while (client.connected() && !client.available()) delay(1);
    while (client.available()) client.read(); // discard request
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.print(webpage);
    client.stop();
  }
  webSocket.loop();
}