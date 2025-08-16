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

// GPIO pins
const int leftBtnPin = 14;     // D5
const int rightBtnPin = 12;    // D6
const int recenterBtnPin = 13; // D7

// Offsets for recentering tilt
float offsetAX = 0, offsetAY = 0;
const float noiseThreshold = 0.15; // deadzone for accelerometer

void calibrateAccel() {
  Serial.println("Calibrating accel... Keep device level");
  const int samples = 200;
  float sumX = 0, sumY = 0;

  for (int i = 0; i < samples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sumX += a.acceleration.x;
    sumY += a.acceleration.y;
    delay(5);
  }

  offsetAX = sumX / samples;
  offsetAY = sumY / samples;

  Serial.print("OffsetAX: "); Serial.println(offsetAX, 3);
  Serial.print("OffsetAY: "); Serial.println(offsetAY, 3);
}

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
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  pinMode(leftBtnPin, INPUT_PULLUP);
  pinMode(rightBtnPin, INPUT_PULLUP);
  pinMode(recenterBtnPin, INPUT_PULLUP);

  calibrateAccel();

  if (!client.connect(host, port)) {
    Serial.println("PC connection failed");
  } else {
    Serial.println("Connected to PC");
  }
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Recenter on button press
  if (digitalRead(recenterBtnPin) == LOW) {
    calibrateAccel();
    delay(500); // prevent multiple recalibrations
  }

  int leftClick = (digitalRead(leftBtnPin) == LOW) ? 1 : 0;
  int rightClick = (digitalRead(rightBtnPin) == LOW) ? 1 : 0;

  // Subtract offsets
  float ax = a.acceleration.x - offsetAX;
  float ay = a.acceleration.y - offsetAY;

  // Apply deadzone
  if (fabs(ax) < noiseThreshold) ax = 0;
  if (fabs(ay) < noiseThreshold) ay = 0;

  // Send accel data instead of gyro
  String data = String(ax, 3) + "," + String(ay, 3) + "," +
                String(leftClick) + "," + String(rightClick) + "\n";
  client.print(data);

  delay(10); // ~100Hz update rate
}
