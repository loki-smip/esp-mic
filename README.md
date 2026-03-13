# 🎙️ ESP8266 Wi-Fi Microphone

Stream audio from an **INMP441 I2S microphone** over Wi-Fi using an **ESP8266**, then receive and transcribe it on your PC in real-time using Python.

---

##  Key Features

-  **UDP Audio Streaming** — Low-latency audio packets sent from ESP8266 to PC over your local network
-  **Arduino OTA (Over-The-Air) Updates** — Flash new firmware to the ESP8266 wirelessly; no USB cable needed after first flash
-  **160 MHz CPU Overclock** — The ESP8266 is set to run at 160 MHz (`system_update_cpu_freq(160)`) for smooth, uninterrupted audio capture and transmission
-  **Software Gain Boosting** — Python-side gain amplifier (`GAIN_FACTOR`) compensates for the INMP441's quiet output; clipping is prevented automatically
-  **Speech-to-Text via Google** — Received audio is automatically transcribed using Google's Speech Recognition API
-  **Mono 16-bit PCM, 16 kHz** — Clean audio format compatible with most speech recognition engines

---

## 🗂️ Project Structure

```
esp/
├── esp8266_mic/
│   └── esp8266_mic.ino      # ESP8266 firmware (I2S capture + UDP send + OTA)
├── python_receiver/
│   ├── main.py              # PC-side receiver (UDP listen + gain boost + transcribe)
│   └── requirements.txt     # Python dependencies
└── README.md
```

---

## 🧰 Hardware Requirements

| Component | Details |
|-----------|---------|
| ESP8266 | NodeMCU / Wemos D1 Mini |
| Microphone | INMP441 (I2S digital mic) |

### INMP441 → ESP8266 Wiring

| INMP441 Pin | ESP8266 Pin |
|-------------|-------------|
| VDD | 3.3V |
| GND | GND |
| WS (LRCK) | GPIO15 (D8) |
| SCK (BCLK) | GPIO14 (D5) |
| SD (Data) | GPIO12 (D6) |
| L/R | GND (for left channel) |

---

## ⚙️ Step-by-Step Setup

### Part 1 — Arduino IDE (ESP8266 Firmware)

#### 1. Install the ESP8266 Board Package
1. Open Arduino IDE → **File → Preferences**
2. Paste this URL into **Additional Boards Manager URLs**:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Go to **Tools → Board → Boards Manager**, search `esp8266`, and install it

#### 2. Install Required Libraries
In Arduino IDE go to **Sketch → Include Library → Manage Libraries** and install:
- `ArduinoOTA` (usually bundled with the ESP8266 core)
- `I2S` (bundled with ESP8266 core — **do not use the Arduino built-in I2S**)

#### 3. Configure Arduino IDE Board Settings

> ⚠️ **CRITICAL — the 160 MHz setting is required for stable audio streaming.**

Go to **Tools** and set the following:

| Setting | Value |
|---------|-------|
| Board | NodeMCU 1.0 (ESP-12E Module) *(or your board)* |
| CPU Frequency | **160 MHz** ← **must be 160 MHz** |
| Upload Speed | 921600 |
| Flash Size | 4MB (FS:2MB OTA:~1019KB) |

#### 4. Edit the Firmware Configuration
Open `esp8266_mic/esp8266_mic.ino` and update these lines:

```cpp
const char* STASSID = "your-ap";         // ← Your Wi-Fi network name (SSID)
const char* STAPSK  = "your_password";   // ← Your Wi-Fi password
const IPAddress listener(192, 168, 1, XX); // ← Your PC's local IP address
const int port = 3333;                   // ← UDP port (keep default or change both sides)
```

> 💡 Find your PC's local IP by running `ipconfig` (Windows) or `ip a` (Linux) in a terminal.

#### 5. Upload via USB (First Time)
- Connect your ESP8266 via USB
- Select the correct **Port** under **Tools → Port**
- Click **Upload** (→)
- Open **Serial Monitor** at **115200 baud** — you should see `Streaming Started...`

#### 6. Upload via OTA (After First Flash)
Once the ESP8266 is connected to Wi-Fi:
1. Under **Tools → Port**, a network port called `esp8266-mic` will appear
2. Select it and click **Upload** as usual — no USB needed!

---

### Part 2 — Python Receiver (PC Side)

#### 1. Find Your Python Version
Make sure Python 3.8+ is installed:
```bash
python --version
```

#### 2. Create a Virtual Environment
> ✅ Always use a virtual environment to avoid dependency conflicts.

```bash
# Navigate to the python_receiver folder
cd esp/python_receiver

# Create the virtual environment
python -m venv venv

# Activate it
# On Windows:
venv\Scripts\activate

# On macOS/Linux:
source venv/bin/activate
```

#### 3. Install Dependencies
```bash
pip install -r requirements.txt
```

#### 4. Edit the Python Configuration
Open `python_receiver/main.py` and check/update these values:

```python
UDP_IP = "0.0.0.0"   # Listen on all interfaces (keep this)
UDP_PORT = 3333       # Must match the port set in the .ino file
SAMPLE_RATE = 16000   # Must match i2s_set_rate() in the .ino (16000)
GAIN_FACTOR = 5.0     # Increase if audio is too quiet (try 3.0–10.0)
```

#### 5. Allow Firewall Access
When you first run the script, Windows Firewall may prompt you — **click "Allow"** to permit UDP traffic on port 3333.

#### 6. Run the Receiver
```bash
python main.py
```

You should see:
```
Listening with 5.0x gain on port 3333...
```

Speak near the INMP441 — transcribed text will print to the console every ~5 seconds.

---

## 🔧 Tuning Tips

| Problem | Fix |
|---------|-----|
| Audio is too quiet | Increase `GAIN_FACTOR` in `main.py` (e.g. `8.0`) |
| Audio is distorted / clipping | Decrease `GAIN_FACTOR` (e.g. `3.0`) |
| Packets dropping / stream stutters | Ensure ESP8266 CPU is set to **160 MHz** in Arduino IDE |
| OTA port not showing | Make sure ESP8266 is on the same Wi-Fi network as your PC; restart Arduino IDE |
| Transcription fails | Check your internet connection (Google Speech API requires internet) |

---

## 📦 Dependencies

See [`python_receiver/requirements.txt`](python_receiver/requirements.txt).

| Package | Purpose |
|---------|---------|
| `sounddevice` | Audio device interface |
| `numpy` | Gain boost math (fast array operations) |
| `SpeechRecognition` | Google Speech-to-Text interface |
| `PyAudio` | Audio I/O backend required by SpeechRecognition |
| `cffi` | C Foreign Function Interface (sounddevice dependency) |

---

## 📝 Notes

- The **160 MHz** CPU frequency is set in firmware via `system_update_cpu_freq(160)` in `setup()`. You **must also set it in Arduino IDE** under **Tools → CPU Frequency** before uploading, or the setting may be overridden by the bootloader.
- Audio is captured as **16-bit signed integers, mono, 16 kHz** — the left I2S channel only (the INMP441's L/R pin is tied to GND).
- The Python script buffers ~5 seconds of audio before transcribing to give the Google API enough context for accurate results.
- OTA hostname is `esp8266-mic` — you can ping it at `esp8266-mic.local` if mDNS is working on your network.

---

## 📄 License

MIT — free to use, modify, and distribute.
