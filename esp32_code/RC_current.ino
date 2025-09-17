#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define ENA 5
#define IN1 18 // Replaced 14
#define IN2 19 // Replaced 27

// === Motor B (Right) ===
#define ENB 23
#define IN3 21 // Replaced 25
#define IN4 22 // Replaced 26

// ====== WiFi credentials (CHANGE THIS to your router/hotspot) ======
const char* ssid = "artifact";       // e.g. Home Router SSID
const char* password = "artifact"; // Router password

// Web servers
WiFiServer server(80);
WebSocketsServer webSocket(81);

// HTML UI (same as before)...
const char webpage[] PROGMEM = R"=====( 
   <!DOCTYPE html>
<html>
<head>
  <title>ESP32 Robot Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { 
      font-family: Arial, sans-serif; 
      text-align: center; 
      margin: 20px;
      background-color: #f0f0f0;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
      background-color: white;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }
    button { 
      padding: 15px 25px; 
      font-size: 16px; 
      margin: 8px; 
      border: none;
      border-radius: 5px;
      cursor: pointer;
      min-width: 200px;
    }
    .stop { background-color: #ff4444; color: white; }
    .cw { background-color: #44aa44; color: white; }
    .ccw { background-color: #4444ff; color: white; }
    .left { background-color: #ff8800; color: white; }
    .right { background-color: #8800ff; color: white; }
    .motor-section { 
      margin: 30px 0; 
      padding: 20px;
      border: 2px solid #ddd;
      border-radius: 8px;
    }
    .slider-container {
      margin: 15px 0;
    }
    input[type="range"] {
      width: 80%;
      height: 25px;
    }
    .status {
      margin: 10px 0;
      padding: 10px;
      border-radius: 5px;
      font-weight: bold;
    }
    .connected { background-color: #d4edda; color: #155724; }
    .disconnected { background-color: #f8d7da; color: #721c24; }
    .keyboard-controls {
      margin: 20px 0;
      padding: 15px;
      background-color: #e9ecef;
      border-radius: 8px;
    }
    .key-info {
      display: inline-block;
      margin: 5px 10px;
      padding: 5px 10px;
      background-color: #6c757d;
      color: white;
      border-radius: 4px;
      font-family: monospace;
      font-weight: bold;
    }
    .active-key {
      background-color: #28a745 !important;
      transform: scale(1.1);
      transition: all 0.1s;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 Robot Controller</h1>
    <div id="status" class="status disconnected">Connecting...</div>

    <div class="keyboard-controls">
      <h3>Keyboard Controls</h3>
      <div>
        <span class="key-info" id="key-w">W</span> Forward
        <span class="key-info" id="key-s">S</span> Backward
        <span class="key-info" id="key-a">A</span> Left Turn
        <span class="key-info" id="key-d">D</span> Right Turn
        <span class="key-info" id="key-space">SPACE</span> Stop
      </div>
      <p><small>Click anywhere on the page first to enable keyboard controls</small></p>
    </div>

    <div class="motor-section">
      <h2>Robot Movement</h2>
      <button class="cw" onclick="sendCommand('robot', 'forward')">FORWARD (W)</button><br>
      <button class="ccw" onclick="sendCommand('robot', 'backward')">BACKWARD (S)</button><br>
      <button class="left" onclick="sendCommand('robot', 'left')">LEFT TURN (A)</button>
      <button class="right" onclick="sendCommand('robot', 'right')">RIGHT TURN (D)</button><br>
      <button class="stop" onclick="sendCommand('robot', 'stop')">STOP ROBOT (SPACE)</button>
      <div class="slider-container">
        <p>Speed: <span id="pwmValueRobot">255</span></p>
        <input type="range" min="0" max="255" value="255" id="pwmSliderRobot" oninput="updatePWM('Robot', this.value)">
      </div>
    </div>

    <button class="stop" onclick="stopAllMotors()" style="font-size: 18px; margin-top: 20px;">STOP ALL MOTORS</button>
  </div>

  <script>
    var connection;
    var pwmValues = { motor1: 255, motor2: 255, robot: 255 };
    var reconnectInterval;
    var pressedKeys = new Set();

    function connectWebSocket() {
      connection = new WebSocket('ws://' + location.hostname + ':81/');
      
      connection.onopen = function() {
        console.log('WebSocket Connected');
        document.getElementById('status').textContent = 'Connected';
        document.getElementById('status').className = 'status connected';
        clearInterval(reconnectInterval);
      };

      connection.onclose = function() {
        console.log('WebSocket Disconnected');
        document.getElementById('status').textContent = 'Disconnected - Reconnecting...';
        document.getElementById('status').className = 'status disconnected';
        
        // Try to reconnect every 3 seconds
        reconnectInterval = setInterval(connectWebSocket, 3000);
      };

      connection.onerror = function(error) {
        console.log('WebSocket Error: ', error);
      };

      connection.onmessage = function(event) {
        console.log('Received: ', event.data);
      };
    }

    function sendCommand(motor, cmd) {
      if(connection && connection.readyState === WebSocket.OPEN) {
        const pwm = pwmValues[motor];
        const message = JSON.stringify({ motor: motor, command: cmd, pwm: pwm });
        connection.send(message);
        console.log('Sent: ', message);
      } else {
        alert('WebSocket not connected!');
      }
    }

    function updatePWM(motorNum, value) {
      if (motorNum === 'Robot') {
        pwmValues['robot'] = parseInt(value);
        document.getElementById('pwmValueRobot').textContent = value;
      } else {
        pwmValues['motor' + motorNum] = parseInt(value);
        document.getElementById('pwmValue' + motorNum).textContent = value;
      }
    }

    function stopAllMotors() {
      sendCommand('motor1', 'stop');
      sendCommand('motor2', 'stop');
      sendCommand('robot', 'stop');
    }

    function highlightKey(key, active) {
      const keyElement = document.getElementById('key-' + key);
      if (keyElement) {
        if (active) {
          keyElement.classList.add('active-key');
        } else {
          keyElement.classList.remove('active-key');
        }
      }
    }

    // Keyboard event handlers
    document.addEventListener('keydown', function(event) {
      const key = event.key.toLowerCase();
      
      // Prevent repeated keydown events when key is held
      if (pressedKeys.has(key)) {
        return;
      }
      pressedKeys.add(key);

      switch(key) {
        case 'w':
          sendCommand('robot', 'forward');
          highlightKey('w', true);
          console.log('Keyboard: Forward (W)');
          break;
        case 's':
          sendCommand('robot', 'backward');
          highlightKey('s', true);
          console.log('Keyboard: Backward (S)');
          break;
        case 'a':
          sendCommand('robot', 'left');
          highlightKey('a', true);
          console.log('Keyboard: Left Turn (A)');
          break;
        case 'd':
          sendCommand('robot', 'right');
          highlightKey('d', true);
          console.log('Keyboard: Right Turn (D)');
          break;
        case ' ':
          event.preventDefault(); // Prevent page scroll
          sendCommand('robot', 'stop');
          highlightKey('space', true);
          console.log('Keyboard: Stop (SPACE)');
          break;
      }
    });

    document.addEventListener('keyup', function(event) {
      const key = event.key.toLowerCase();
      pressedKeys.delete(key);

      switch(key) {
        case 'w':
          highlightKey('w', false);
          break;
        case 's':
          highlightKey('s', false);
          break;
        case 'a':
          highlightKey('a', false);
          break;
        case 'd':
          highlightKey('d', false);
          break;
        case ' ':
          highlightKey('space', false);
          break;
      }
    });

    // Make sure the page can receive keyboard events
    // document.addEventListener('click', function() {
    //   document.body.focus();
    // });

    // Connect when page loads
    window.onload = function() {
      connectWebSocket();
      // Make page focusable for keyboard events
      document.body.setAttribute('tabindex', '0');
      document.body.focus();
    };
  </script>
</body>
</html>
)=====";

// ========== Motor Functions (same as before) ==========
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
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void right_turn(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
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

  // ====== WiFi in Station Mode ======
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

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
