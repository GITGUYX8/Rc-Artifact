# RC-Artifact – AR-Enabled RC Car

**RC-Artifact** is an augmented reality–enabled remote control car powered by an **ESP32 microcontroller**, controlled through **WebSockets**, and visualized via **scrcpy screen casting**.  
It combines **IoT + AR + custom desktop controls** into one seamless project.  

---

##  Features

-  **ElectronJS Desktop App**  
  - Connects to ESP32 using its Wi-Fi **STA mode IP**  
  - Provides **custom RC controls** to drive the car  
  - Directly launches **scrcpy** for real-time phone screen casting  

-  **ESP32 + WebSockets**  
  - ESP32 connects to your Wi-Fi in **Station (STA) mode**  
  - Receives control signals from the Electron app  
  - Sends commands to the **L298 motor driver** to move BO motors  

-  **Hardware Control**  
  - **L298 Motor Driver** for DC BO motors  
  - Forward / Backward / Left / Right driving  
  - Smooth real-time WebSocket-based communication  

-  **AR Visualization**  
  - Mount your phone on the RC car  
  - Stream live feed with **scrcpy** on the desktop  
  - Combine AR effects with control panel  

---

##  Hardware Requirements

-  ESP32 (with Wi-Fi enabled)  
-  L298N Motor Driver  
-  BO Motors (x4) + Wheels  
-  Android Phone (for AR camera feed via scrcpy)  
-  Chassis for assembly  
-  Power Supply (Li-ion / LiPo battery recommended)  

---

## Software Requirements

- [Node.js](https://nodejs.org/) (for Electron app)  
- [ElectronJS](https://www.electronjs.org/)  
- [scrcpy](https://github.com/Genymobile/scrcpy) (for phone screen casting)  
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)  

---

## Getting Started

### Setup ESP32 Firmware
- Configure ESP32 in **STA Mode** with your Wi-Fi credentials  
- Flash the firmware that sets up a **WebSocket server**  
- Note down the **IP address** assigned by your Wi-Fi  

### Setup Desktop App
```bash
# Clone the repository
git clone https://github.com/your-username/rc-artifact.git
cd rc-artifact

# Install dependencies
npm install

# Run the app
npm start
