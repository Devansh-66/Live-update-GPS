# Live GPS Tracker Using ESP8266

This project is a real-time GPS tracker that uses the ESP8266 microcontroller to fetch location data from a GPS module, display it on a web interface, and broadcast updates to connected clients via WebSockets.

## Features
- Real-time GPS tracking with live updates on a web interface.
- Dynamic Google Maps link for easy location visualization.
- WebSocket-based communication for fast and efficient data updates.
- Simple WiFi configuration using ESPAsyncWiFiManager.
- LED status indicator for connection state.

## Hardware Requirements
- ESP8266 (e.g., NodeMCU).
- GPS module (e.g., Neo-6M).
- Connecting wires (e.g., Jumper wires)
- 3.7V Li-ion battery.

## Circuit Diagram
| Pin Name       | ESP8266 Pin |
|----------------|-------------|
| RX (GPS)       | D7          |
| TX (GPS)       | D6          |
| Power Supply   | 3V, GND     |
| LED            | D4          |

### Circuit Details
- Connect the **RX** pin of the GPS module to the **TX (D7)** pin of the ESP8266 and the **TX** pin of the GPS module to the **RX (D6)** pin of the ESP8266.  
- Power the GPS module and the ESP8266 using a 3.7V Li-ion battery, connected to the **3V** and **GND** pins of the ESP8266.  
- An inbuilt LED directly connected to the **D4** pin of the ESP8266 to indicate the WiFi connection status.  

## How It Works
1. **WiFi Configuration**:
   - The device starts in AP mode if it fails to connect to a known WiFi network.
   - Connect to the ESP8266’s hotspot (default name: `ESP_WiFi`) to configure your WiFi credentials.

2. **GPS Data Fetching**:
   - The GPS module sends location data to the ESP8266 via serial communication.
   - Data such as latitude, longitude, altitude, speed, course, hdop , date and time are extracted and stored.

3. **Web Interface**:
   - A responsive web page hosted by the ESP8266 displays the current location and movement data.
   - Google Maps integration provides easy navigation and visualization.

4. **WebSocket Updates**:
   - Real-time updates are broadcast to all connected clients via WebSockets.

## Usage
1. Power on the device using a 3.7V Li-ion battery.
2. Connect the device to your WiFi network (if not already configured).
3. Access the web interface using the IP address shown on the serial monitor.
4. View real-time GPS data and track your device on Google Maps.

## LED Status
- **Slow Blink**: Connected to WiFi.
- **Fast Blink**: Disconnected from WiFi or in AP mode.

## Example Web Interface
![Web Interface](https://github.com/user-attachments/assets/05f42742-a1ca-4c04-8c36-21dac5e5571d)


## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
