#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define pins for ultrasonic sensor
#define TRIG_PIN 2
#define ECHO_PIN 3

// Define pins for relay and buzzer
#define RELAY_PIN 13



// Hysteresis thresholds
const float relayOnThreshold = 25.0;  // Percent at which relay turns on
const float relayOffThreshold = 90.0; // Percent at which relay turns off

// Variables for smoothing
const int numReadings = 5; // Number of readings for smoothing
int readings[numReadings];  // Array to store readings
int readIndex = 0;          // Index for the current reading
int total = 0;              // Sum of readings
int smoothedWaterLevel = 0; // Smoothed water level

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(1000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  // Initialize smoothing array
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
}

void loop() {
  // Trigger the ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the response time
  float duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance (cm)
  float distance = (duration / 2) / 29.1;

  // Map the distance to water level percentage
  int currentWaterLevel = map(distance, 20, 40, 100, 0);

  // Constrain the water level to 0-100%
  currentWaterLevel = constrain(currentWaterLevel, 0, 100);

  // Update smoothing array
  total = total - readings[readIndex];       // Subtract the oldest reading
  readings[readIndex] = currentWaterLevel;  // Store the new reading
  total = total + readings[readIndex];      // Add the new reading
  readIndex = (readIndex + 1) % numReadings; // Advance to the next index

  // Calculate the smoothed water level
  smoothedWaterLevel = total / numReadings;

  // Display the smoothed water level on the OLED
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(30, 25);
  display.print(smoothedWaterLevel);
  display.print(" %");
  display.display();

  // Print water level to Serial Monitor
  Serial.print("Water Level: ");
  Serial.print(smoothedWaterLevel);
  Serial.println("%");

  // Control the relay using hysteresis
  if (smoothedWaterLevel < relayOnThreshold) {
    digitalWrite(RELAY_PIN, LOW); // Turn the relay on
  } else if (smoothedWaterLevel > relayOffThreshold) {
    digitalWrite(RELAY_PIN, HIGH);  // Turn the relay off
  }

  delay(500); // Delay for stability
}
