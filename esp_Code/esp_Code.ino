#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "VARAD";
const char* password = "Adventure4@4242";
const char* host = "192.168.1.26"; // Your PC IP
const uint16_t port = 5005;

WiFiClient client;
Adafruit_MPU6050 mpu;

const int leftBtnPin = 14;    // D5
const int rightBtnPin = 12;   // D6
const int recenterBtnPin = 13; // D7

float offsetX = 0, offsetY = 0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050");
    while (1);
  }

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  pinMode(leftBtnPin, INPUT_PULLUP);
  pinMode(rightBtnPin, INPUT_PULLUP);
  pinMode(recenterBtnPin, INPUT_PULLUP);

  if (!client.connect(host, port)) {
    Serial.println("PC connection failed");
  } else {
    Serial.println("Connected to PC");
  }
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if (digitalRead(recenterBtnPin) == LOW) {
    offsetX = g.gyro.x;
    offsetY = g.gyro.y;
  }

  int leftClick = (digitalRead(leftBtnPin) == LOW) ? 1 : 0;
  int rightClick = (digitalRead(rightBtnPin) == LOW) ? 1 : 0;

  // Send gyro data minus offset for recentering
  String data = String(g.gyro.x - offsetX) + "," + 
                String(g.gyro.y - offsetY) + "," + 
                String(leftClick) + "," + 
                String(rightClick) + "\n";
  client.print(data);

  delay(10); // ~100Hz
}
