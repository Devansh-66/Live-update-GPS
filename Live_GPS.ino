#include <ESPAsyncWiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// Pin configuration
static const int RXpin = D7, TXpin = D6;
const int ledPin = D4;

// GPS and WiFiManager objects
TinyGPSPlus gps;
SoftwareSerial SerialGPS(TXpin, RXpin);
DNSServer dns;
AsyncWebServer server(80);
AsyncWiFiManager wifiManager(&server, &dns);

// WebSocket
AsyncWebSocket ws("/ws");

// Variables for GPS data
float Latitude = 0, Longitude = 0, Altitude = 0, Speed = 0, HDOP = 0, Course = 0;
int year, month, date, hour, minute, second;
String DateString, TimeString;

// Timer for loop execution
unsigned long lastBroadcastTime = 0;
const unsigned long broadcastInterval = 1000;  // 1 second
// Timer for LED blinking
unsigned long previousMillis = 0;
const unsigned long ledOnDuration = 1000;               // 1 second
const unsigned long ledOffDurationConnected = 1000;     // 4 seconds (slow blink for connected)
const unsigned long ledOffDurationDisconnected = 4000;  // 1 second (fast blink for disconnected)
bool ledState = false;

// WebSocket event handler
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");

  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  }
}

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);  // Turn off LED

  // Setup WiFi
  wifiManager.autoConnect("ESP_WiFi");
  Serial.println("Connected to WiFi!");

  // Setup WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  // Serve HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String mapUrl = "https://maps.google.com/?q=" + String(Latitude, 6) + "," + String(Longitude, 6);
    request->send_P(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>Live GPS Tracking</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          background-color: #f4f4f4;
          margin: 0;
          padding: 20px;
        }
        .container {
          max-width: 800px;
          margin: 0 auto;
          background-color: white;
          border-radius: 15px;
          box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
          padding: 20px;
          display: flex;
          flex-direction: column;
        }
        .header {
          text-align: center;
          margin-bottom: 20px;
        }
        .header h1 {
          color: #333;
          font-size: 24px;
          margin-bottom: 10px;
        }
        .header p {
          color: #666;
        }
        .card {
          flex: 1;
          background-color: #f0f0f0;
          border-radius: 10px;
          padding: 15px;
          margin: 10px;
        }
        .card-title {
          color: #007bff;
          font-weight: bold;
          margin-bottom: 10px;
        }
        .row {
          display: flex;
          align-items: center;
          margin-bottom: 10px;
        }
        .icon {
          margin-right: 10px;
          font-size: 18px;
        }
        .label {
          font-weight: bold;
          margin-right: 10px;
          color: #333;
        }
        .footer {
          text-align: center;
          background-color: #f1f1f1;
          padding: 10px;
          margin-top: 20px;
        }
        .socialicon {
          max-width: 35px;
          max-height: 35px;
          padding-left: 24px;
          padding-right: 24px;
        }
        @media (max-width: 768px) {
          .container {
            flex-direction: column;
          }
        }
      </style>
      <script>
        var ws = new WebSocket("ws://" + location.hostname + "/ws");
        ws.onmessage = function(event) {
          var data = JSON.parse(event.data);
          document.getElementById("latitude").textContent = data.latitude || "N/A";
          document.getElementById("longitude").textContent = data.longitude || "N/A";
          document.getElementById("altitude").textContent = data.altitude || "N/A";
          document.getElementById("speed").textContent = data.speed || "N/A";
          document.getElementById("hdop").textContent = data.hdop || "N/A";
          document.getElementById("course").textContent = data.course || "N/A";
          document.getElementById("date").textContent = data.date || "N/A";
          document.getElementById("time").textContent = data.time || "N/A";

          // Update Google Maps link dynamically
          var mapUrl = "https://maps.google.com/?q=" + data.latitude + "," + data.longitude;
          document.getElementById("mapLink").setAttribute("href", mapUrl);
        };
      </script>
    </head>
    <body>
      <div class="container">

        <!-- Header -->
        <div class="header">
          <h1>Live GPS Tracking</h1>
          <p>Real-time Location Monitoring</p>
        </div>

        <!-- Location Details -->
        <div class="card">
          <div class="card-title">Location Details</div>
          <div class="row">
            <div class="icon">🌍</div>
            <div class="label">Latitude:</div>
            <span id="latitude">N/A</span>°N
          </div>
          <div class="row">
            <div class="icon">🌎</div>
            <div class="label">Longitude:</div>
            <span id="longitude">N/A</span>°E
          </div>
          <div class="row">
            <div class="icon">🌏</div>
            <div class="label">Altitude:</div>
            <span id="altitude">N/A</span> m
          </div>

          <!-- Map Link -->
          <div class="row">
            <a id="mapLink" href="#" target="_blank">Click to view on Google Maps</a>
          </div>
        </div>

        <!-- Movement Details -->
        <div class="card">
          <div class="card-title">Movement & Environment</div>
          <div class="row">
            <div class="icon">🧭</div>
            <div class="label">Course:</div>
            <span id="course">N/A</span>°
          </div>
          <div class="row">
            <div class="icon">🚗</div>
            <div class="label">Speed:</div>
            <span id="speed">N/A</span> km/h
          </div>
          <div class="row">
            <div class="icon">📡</div>
            <div class="label">HDOP:</div>
            <span id="hdop">N/A</span>
          </div>
        </div>

        <!-- Date and Time -->
        <div class="footer">
          📅 <span id="date">N/A</span>
          | 🕒 
          <span id="time">N/A</span>
        </div>

        <!-- Footer -->
        <div class="footer">
          | Made by: Devansh Rajput |
        </div>

        <!-- Social Links -->
        <div class="footer">
          <a href="https://github.com/Devansh-66" target="_blank"><img class="socialicon" src="https://cdn2.iconfinder.com/data/icons/social-icons-circular-color/512/github-1024.png"></a>
          <a href="https://www.linkedin.com/in/devansh-rajput" target="_blank"><img class="socialicon" src="https://cdn4.iconfinder.com/data/icons/social-media-logos-6/512/56-linkedin-1024.png"></a>
          <a href="https://www.instagram.com/_devansh.rajput_/" target="_blank"><img class="socialicon" src="https://cdn4.iconfinder.com/data/icons/social-media-logos-6/512/62-instagram-1024.png"></a>
        </div>

      </div> <!-- container close -->
    </body>
    </html>
  )rawliteral");
  });


  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Check WiFi

  unsigned long currentMillis = millis();

  if (WiFi.status() == WL_CONNECTED) {
    // Handle slow blink (connected)
    if (!ledState && currentMillis - previousMillis >= ledOffDurationConnected) {
      digitalWrite(ledPin, HIGH);  // Turn LED on
      ledState = true;
      previousMillis = currentMillis;
    } else if (ledState && currentMillis - previousMillis >= ledOnDuration) {
      digitalWrite(ledPin, LOW);  // Turn LED off
      ledState = false;
      previousMillis = currentMillis;
    }

    // Broadcast GPS data every second
    if (millis() - lastBroadcastTime >= broadcastInterval) {
      lastBroadcastTime = millis();

      // Create JSON object
      StaticJsonDocument<256> jsonDoc;
      jsonDoc["latitude"] = Latitude;
      jsonDoc["longitude"] = Longitude;
      jsonDoc["altitude"] = Altitude;
      jsonDoc["speed"] = Speed;
      jsonDoc["hdop"] = HDOP;
      jsonDoc["course"] = Course;
      jsonDoc["date"] = DateString;
      jsonDoc["time"] = TimeString;

      String jsonString;
      serializeJson(jsonDoc, jsonString);

      // Send JSON to all WebSocket clients
      ws.textAll(jsonString);
    }
  } else {
    // Handle fast blink (disconnected)
    if (!ledState && currentMillis - previousMillis >= ledOffDurationDisconnected) {
      digitalWrite(ledPin, HIGH);  // Turn LED on
      ledState = true;
      previousMillis = currentMillis;
    } else if (ledState && currentMillis - previousMillis >= ledOnDuration) {
      digitalWrite(ledPin, LOW);  // Turn LED off
      ledState = false;
      previousMillis = currentMillis;
    }

    // Start Access Point if disconnected
    Serial.println("WiFi disconnected! Starting Access Point...");
    wifiManager.startConfigPortal("ESP_Hotspot");
    delay(1000);  // Avoid rapid reconnect attempts
  }
  // Fetch GPS data
  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      Latitude = gps.location.isValid() ? gps.location.lat() : 0.0;
      Longitude = gps.location.isValid() ? gps.location.lng() : 0.0;
      Altitude = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
      Speed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
      HDOP = gps.hdop.isValid() ? gps.hdop.value() / 100.0 : 0.0;
      Course = gps.course.isValid() ? gps.course.deg() : 0.0;

      // Print components for serial monitor

      Serial.print("Latitude: ");
      Serial.println(Latitude, 6);
      Serial.print("Longitude: ");
      Serial.println(Longitude, 6);
      Serial.print("Altitude: ");
      Serial.println(Altitude, 2);
      Serial.print("Speed: ");
      Serial.println(Speed, 6);
      Serial.print("hdop: ");
      Serial.println(HDOP, 2);
      Serial.print("Course: ");
      Serial.println(Course, 2);

      if (gps.date.isValid()) {
        DateString = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();

        // Add leading zero to day if necessary
        if (date < 10) {
          DateString += '0';
        }
        DateString += String(date);
        DateString += " / ";

        // Add leading zero to month if necessary
        if (month < 10) {
          DateString += '0';
        }
        DateString += String(month);
        DateString += " / ";

        DateString += String(year);

        // Print components for serial monitor
        Serial.print("Date: ");
        Serial.println(date);
        Serial.print("Month: ");
        Serial.println(month);
        Serial.print("Year: ");
        Serial.println(year);
      }

      if (gps.time.isValid()) {
        TimeString = "";
        hour = gps.time.hour() + 5;  // Adjust UTC offset
        minute = gps.time.minute() + 30;
        second = gps.time.second();

        // Handle minute overflow
        if (minute >= 60) {
          hour += 1;
          minute -= 60;
        }

        // Handle hour overflow
        if (hour >= 24) {
          hour -= 24;
        }

        // Add leading zero to hour if necessary
        if (hour < 10) {
          TimeString += '0';
        }
        TimeString += String(hour);
        TimeString += " : ";

        // Add leading zero to minute if necessary
        if (minute < 10) {
          TimeString += '0';
        }
        TimeString += String(minute);
        TimeString += " : ";

        // Add leading zero to second if necessary
        if (second < 10) {
          TimeString += '0';
        }
        TimeString += String(second);

        // Print components for serial monitor
        Serial.print("Hour: ");
        Serial.println(hour);
        Serial.print("Minute: ");
        Serial.println(minute);
        Serial.print("Second: ");
        Serial.println(second);
      }

      if (!gps.date.isValid() || !gps.time.isValid()) {
        DateString = "N/A";
        TimeString = "N/A";
      }
    }
  }

  // Clean up disconnected WebSocket clients
  ws.cleanupClients();
}
