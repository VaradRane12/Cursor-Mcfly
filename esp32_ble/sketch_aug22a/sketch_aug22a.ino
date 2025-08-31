#include "BleMouse.h"
#include <Wire.h>
#include <MPU6050.h>

// Create a BluetoothMouse object
BleMouse bleMouse;

// MPU6050 object
MPU6050 gyro;

// Sensitivity for cursor movement
float sensitivity = 1.0;  // Lower for more precision

// Variables for accelerometer readings and angles
float axx = 0;
float ayy = 0;
float azz = 0;
float roll = 0;   // Angle around X-axis
float pitch = 0;  // Angle around Y-axis

// Kalman filter variables
float roll_est = 0, roll_err = 1;
float pitch_est = 0, pitch_err = 1;
float process_noise = 0.01;
float measurement_noise = 0.1;

// Calibration offsets (baseline for reset)
float ay_offset = 0.0;
float roll_offset = 0.0;
float pitch_offset = 0.0;

// Button pin setup
const int leftButtonPin  = 15;  // Left click
const int rightButtonPin = 4;   // Right click
const int resetButtonPin = 2;   // Reset calibration

bool lastLeftState  = HIGH;
bool lastRightState = HIGH;
bool lastResetState = HIGH;

// For smoothing
float prevDeltaX = 0;
float prevDeltaY = 0;
float alpha = 0.3;  // Smoothing factor (0.1 = heavy smoothing, 0.5 = light)

void setup() {
  Serial.begin(115200);

  Wire.begin();
  gyro.initialize();

  // Calibration for Y offset
  int numSamples = 100;
  int16_t ax, ay, az;
  for (int i = 0; i < numSamples; i++) {
    gyro.getAcceleration(&ax, &ay, &az);
    ay_offset += ay / 16384.0;
    delay(10);
  }
  ay_offset /= numSamples;

  bleMouse.begin();
  Serial.println("Bluetooth Mouse is ready to pair");

  // Setup button pins as input with pull-up
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
}

float kalmanFilter(float measurement, float &estimate, float &error) {
  error += process_noise;
  float kalman_gain = error / (error + measurement_noise);
  estimate += kalman_gain * (measurement - estimate);
  error *= (1 - kalman_gain);
  return estimate;
}

void loop() {
  if (bleMouse.isConnected()) {
    // Read accelerometer data
    int16_t ax, ay, az;
    gyro.getAcceleration(&ax, &ay, &az);

    axx = ax / 16384.0;
    ayy = (ay / 16384.0) - ay_offset;
    azz = az / 16384.0;

    // Calculate roll and pitch
    float roll_means = atan2(ayy, sqrt(axx * axx + azz * azz)) * 180 / PI;
    float pitch_means = atan2(-axx, sqrt(ayy * ayy + azz * azz)) * 180 / PI;

    // Apply Kalman filter
    roll = kalmanFilter(roll_means, roll_est, roll_err);
    pitch = kalmanFilter(pitch_means, pitch_est, pitch_err);

    // Apply baseline offsets
    roll -= roll_offset;
    pitch -= pitch_offset;

    // Map pitch and roll to mouse cursor movement
    int deltaY = roll * sensitivity;   // Roll controls horizontal
    int deltaX = pitch * sensitivity;  // Pitch controls vertical

    // Dead zone
    if (abs(deltaX) < 2) deltaX = 0;
    if (abs(deltaY) < 2) deltaY = 0;

    // Smoothing
    deltaX = alpha * deltaX + (1 - alpha) * prevDeltaX;
    deltaY = alpha * deltaY + (1 - alpha) * prevDeltaY;
    prevDeltaX = deltaX;
    prevDeltaY = deltaY;

    // Limit max movement
    deltaX = constrain(deltaX, -20, 20);
    deltaY = constrain(deltaY, -20, 20);

    // Move the cursor
    bleMouse.move(deltaX, -deltaY);

    // Debug output
    Serial.print("Roll: ");
    Serial.print(roll);
    Serial.print(" | Pitch: ");
    Serial.print(pitch);
    Serial.print(" | DeltaX: ");
    Serial.print(deltaX);
    Serial.print(" | DeltaY: ");
    Serial.println(deltaY);

    // LEFT click
    bool currentLeftState = digitalRead(leftButtonPin);
    if (lastLeftState == HIGH && currentLeftState == LOW) {
      bleMouse.click(MOUSE_LEFT);
      Serial.println("Left Click!");
    }
    lastLeftState = currentLeftState;

    // RIGHT click
    bool currentRightState = digitalRead(rightButtonPin);
    if (lastRightState == HIGH && currentRightState == LOW) {
      bleMouse.click(MOUSE_RIGHT);
      Serial.println("Right Click!");
    }
    lastRightState = currentRightState;

    // RESET calibration (baseline reset)
  bool currentResetState = digitalRead(resetButtonPin);
if (lastResetState == HIGH && currentResetState == LOW) {
    bleMouse.click(MOUSE_MIDDLE);
    Serial.println("Middle Click!");
}
lastResetState = currentResetState;


    delay(10);  // Smoother loop
  } else {
    Serial.println("Mouse not connected");
    delay(1000);
  }
}
