# M5StickC Plus OBS Controller

A simple OBS (Open Broadcaster Software) controller interface built using the M5StickC Plus. This project allows you to control OBS from your M5StickC Plus device over Wi-Fi, with features including switching scenes, toggling streaming status, and displaying the current scene and stream status.

## Features

- **Wi-Fi Connectivity:** Connects to your local Wi-Fi network to interact with OBS over WebSocket.
- **Scene Switching:** Toggle between scenes in OBS using the M5StickC Plus buttons.
- **Streaming Status:** View and toggle the stream status directly from the M5StickC Plus display.
- **OBS Authentication:** Secure WebSocket connection to OBS using authentication.

## Requirements

- **M5StickC Plus:** The hardware used to interface with OBS.
- **Wi-Fi Network:** The M5StickC Plus must be connected to a Wi-Fi network to communicate with OBS.
- **OBS WebSocket Plugin:** The WebSocket plugin for OBS must be installed and configured on your OBS instance.
- **Arduino IDE:** Used to compile and upload the code to the M5StickC Plus.

## Installation

### 1. Install the required libraries in the Arduino IDE:

- **M5StickC Plus:** This library is used for controlling the M5StickC Plus hardware.
- **ArduinoWebsockets:** A library to manage WebSocket communication.
- **ArduinoJson:** For parsing and constructing JSON objects for WebSocket communication.
- **mbedtls:** Used for hashing and authentication with OBS.

### 2. Install the **OBS WebSocket Plugin**:

Download and install the [OBS WebSocket plugin](https://github.com/obsproject/obs-websocket) for OBS. Ensure that the plugin is enabled and configure it to allow remote connections.

### 3. Update the code with your Wi-Fi and OBS settings:

- Replace the `ssid` and `password` variables with your Wi-Fi credentials.
- Set `obsHost` to the hostname or IP address of your OBS instance (e.g., `raspberrypi.local` or `192.168.x.x`).
- Set `obsPort` to the WebSocket port of your OBS instance (default is 4455).
- Set `obsPassword` to the password you’ve configured for OBS WebSocket authentication.

### 4. Upload the code to your M5StickC Plus:

- Open the Arduino IDE.
- Select the correct board (`M5StickC Plus`).
- Upload the code to your M5StickC Plus device.

## Usage

1. **Wi-Fi Connection:**
   Once the M5StickC Plus is powered on, it will attempt to connect to the Wi-Fi network using the provided credentials.

2. **OBS WebSocket Connection:**
   After connecting to Wi-Fi, the M5StickC Plus will connect to the OBS WebSocket instance. The screen will display the connection status.

3. **Scene Switching:**
   Press **Button B** on the M5StickC Plus to cycle through the scenes in OBS.

4. **Streaming Control:**
   Press **Button A** to toggle the streaming status on or off. The streaming status will be displayed on the screen.

5. **Display:**
   The M5StickC Plus will display:
   - Wi-Fi connection status (`OK` or `NO`).
   - OBS connection status (`OK` or `NO`).
   - Current stream status (`ON` or `OFF`).
   - Current scene name.

## Code Overview

The code connects to OBS over WebSocket using the `ArduinoWebsockets` library and listens for events. It then interacts with the OBS WebSocket API to control scenes and streaming.

- **Wi-Fi Setup:** The device connects to your Wi-Fi network using the provided credentials.
- **WebSocket Connection:** Once connected to Wi-Fi, the M5StickC Plus connects to OBS via WebSocket, authenticates using a challenge-response method, and listens for status updates.
- **Scene Switching and Streaming:** Buttons A and B are used to control the stream and switch scenes in OBS.

## Example Output

The M5StickC Plus will display the following on its screen:
WiFi: OK OBS: OK Stream: OFF Scene: Loading...


After a button press, the display will update accordingly:
WiFi: OK OBS: OK Stream: ON Scene: Scene1


## Troubleshooting

- **Wi-Fi Connection:** Ensure your M5StickC Plus is within range of your Wi-Fi network and that the correct SSID and password are provided.
- **OBS WebSocket Plugin:** Ensure the OBS WebSocket plugin is installed and configured with the correct port and password.
- **OBS Authentication:** If you're receiving authentication errors, check the WebSocket plugin settings and ensure the password is correct.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


