#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* wifiName = "JAL";
const char* wifiPassword = "JoelAlveLeo020609";

// Stepper motor pin setup
const int stepperMotorPins[4] = {13, 14, 15, 16};  // IN1 to IN4 of ULN2003
const int totalStepPositions = 8;
const int fullRotationStepCount = 4096; // For 28BYJ-48 stepper motor

// Motor stepping sequence for half-step mode
const int motorStepPattern[totalStepPositions][4] = {
  {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0},
  {0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 1}, {1, 0, 0, 1}
};

WebServer httpServer(80);

volatile bool isManualMode = false;
bool hasAutoRotated = false;
int currentStepIndex = 0;
unsigned long lastLightSensorCheckTime = 0;
const unsigned long lightSensorCheckInterval = 500; // in milliseconds

// Light sensor setup
const int lightSensorPin = 1;
const int lightThreshold = 20;

void initializeMotorPins() {
  for (int i = 0; i < 4; i++) {
    pinMode(stepperMotorPins[i], OUTPUT);
    digitalWrite(stepperMotorPins[i], LOW);
  }
}

void energizeMotorStep(int stepIndex) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(stepperMotorPins[i], motorStepPattern[stepIndex][i]);
  }
}

void rotateMotorBySteps(int numberOfSteps, int direction) {
  for (int i = 0; i < numberOfSteps; i++) {
    currentStepIndex = (currentStepIndex + direction + totalStepPositions) % totalStepPositions;
    energizeMotorStep(currentStepIndex);
    delay(2); // Delay between steps
  }
  disableMotor();
}

void disableMotor() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(stepperMotorPins[i], LOW);
  }
}

// HTTP endpoint handlers
void handleMoveUp() {
  isManualMode = true;
  rotateMotorBySteps(10, 1); // Steps, direction 
  Serial.println("Manual command: move up");
  httpServer.send(200, "text/plain", "Moved up by two steps manually.");
}

void handleMoveDown() {
  isManualMode = true;
  rotateMotorBySteps(10, -1);
  Serial.println("Manual command: move down");
  httpServer.send(200, "text/plain", "Moved down by two steps manually.");
}

void handleManualStop() {
  isManualMode = false;
  disableMotor();
  Serial.println("Manual command: stop motor");
  httpServer.send(200, "text/plain", "Motor stopped. Resuming automatic control.");
}

void handleUnknownRoute() {
  Serial.println("HTTP command not recognized");
  httpServer.send(404, "text/plain", "Command not found.");
}

void connectToWiFiNetwork() {
  Serial.print("Connecting to WiFi network: ");
  Serial.println(wifiName);
  WiFi.begin(wifiName, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);
  initializeMotorPins();
  connectToWiFiNetwork();

  httpServer.on("/up", handleMoveUp);
  httpServer.on("/down", handleMoveDown);
  httpServer.on("/stop", handleManualStop);
  httpServer.onNotFound(handleUnknownRoute);
  httpServer.begin();

  analogReadResolution(12);  // Set ADC resolution to 12 bits
}

void loop() {
  httpServer.handleClient();

  if (!isManualMode && millis() - lastLightSensorCheckTime >= lightSensorCheckInterval) {
    lastLightSensorCheckTime = millis();
    int lightLevel = analogRead(lightSensorPin);
    Serial.print("Light Sensor Value: ");
    Serial.println(lightLevel);

    if (lightLevel > lightThreshold && !hasAutoRotated) {
      rotateMotorBySteps(fullRotationStepCount, 1);
      hasAutoRotated = true;
    } else if (lightLevel < lightThreshold && hasAutoRotated) {
      rotateMotorBySteps(fullRotationStepCount, -1);
      hasAutoRotated = false;
    }
  }
}
