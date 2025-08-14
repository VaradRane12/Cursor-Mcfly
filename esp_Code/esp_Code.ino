#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* host = "192.168.1.100"; // Your PC IP
const uint16_t port = 5005;

WiFiClient client;
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1);
  }

  if (!client.connect(host, port)) {
    Serial.println("Connection to PC failed!");
  } else {
    Serial.println("Connected to PC server");
  }
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Send accel X/Y as mouse movement
  String data = String(a.acceleration.x) + "," + String(a.acceleration.y);
  client.println(data);
  delay(10);
}
