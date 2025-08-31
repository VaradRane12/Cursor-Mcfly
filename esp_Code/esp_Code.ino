#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

const char* ssid = "VARAD";
const char* password = "Adventure4@4242";
const char* host = "192.168.1.26"; // Your PC IP
const uint16_t port = 5005;

WiFiClient client;
Adafruit_MPU6050 mpu;

// GPIO pins
const int leftBtnPin = 14;     // D5
const int rightBtnPin = 12;    // D6
const int recenterBtnPin = 13; // D7

// Complementary filter
float fusedX = 0, fusedY = 0;
unsigned long lastTime = 0;

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

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  pinMode(leftBtnPin, INPUT_PULLUP);
  pinMode(rightBtnPin, INPUT_PULLUP);
  pinMode(recenterBtnPin, INPUT_PULLUP);

  lastTime = millis();

  if (!client.connect(host, port)) {
    Serial.println("PC connection failed");
  } else {
    Serial.println("Connected to PC");
  }
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;

  // --- Calculate tilt from accelerometer ---
  float accAngleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  float accAngleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * 180 / PI;

  // --- Integrate gyro for relative angles (deg/s * time) ---
  static float gyroAngleX = 0, gyroAngleY = 0;
  gyroAngleX += g.gyro.x * dt * 180 / PI;
  gyroAngleY += g.gyro.y * dt * 180 / PI;

  // --- Complementary filter fusion ---
  const float alpha = 0.98; // 98% gyro, 2% accel
  fusedX = alpha * (fusedX + g.gyro.x * dt * 180 / PI) + (1 - alpha) * accAngleX;
  fusedY = alpha * (fusedY + g.gyro.y * dt * 180 / PI) + (1 - alpha) * accAngleY;

  // --- Button handling ---
  int leftClick = (digitalRead(leftBtnPin) == LOW) ? 1 : 0;
  int rightClick = (digitalRead(rightBtnPin) == LOW) ? 1 : 0;

  // --- Send fused angles ---
  String data = String(fusedX, 3) + "," + String(fusedY, 3) + "," +
                String(leftClick) + "," + String(rightClick) + "\n";
  client.print(data);

  delay(1); // ~100Hz
}
