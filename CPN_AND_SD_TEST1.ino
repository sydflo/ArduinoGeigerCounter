#include <SD.h>

#define LOG_PERIOD 15000     // Logging period in ms (15 seconds)
#define MAX_PERIOD 60000     // Used to calculate CPM multiplier
#define PULSE_PIN 2          // Geiger counter pulse signal input
#define SD_CS_PIN 4          // SD card module CS (Chip Select) pin

unsigned long counts = 0;       // Pulse count within the log period
unsigned long cpm = 0;          // Counts per minute
unsigned int multiplier;        // CPM multiplier = MAX_PERIOD / LOG_PERIOD
unsigned long previousMillis = 0;

File logFile;

// Interrupt service routine (ISR) for Geiger pulse
void tube_impulse() {
  counts++;
}

void setup() {
  Serial.begin(9600);

  // Set up Geiger pulse pin
  pinMode(PULSE_PIN, INPUT);  // Input from Geiger OUT
  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), tube_impulse, FALLING);

  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    while (1);  // Halt if SD card doesn't start
  }
  Serial.println("SD card initialized.");

  // Create the CSV file if it doesn't exist
  logFile = SD.open("geiger.csv", FILE_WRITE);
  if (logFile) {
    logFile.println("Timestamp(ms),CPM");
    logFile.close();
    Serial.println("Logging to geiger.csv started.");
  } else {
    Serial.println("Error creating geiger.csv!");
    while (1);  // Halt if file can't be opened
  }

  multiplier = MAX_PERIOD / LOG_PERIOD;
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis > LOG_PERIOD) {
    previousMillis = currentMillis;
    cpm = counts * multiplier;

    // Print to Serial Monitor
    Serial.print("CPM: ");
    Serial.print(cpm);
    Serial.print(" at ");
    Serial.println(currentMillis);

    // Log to SD card
    logFile = SD.open("geiger.csv", FILE_WRITE);
    if (logFile) {
      logFile.print(currentMillis);
      logFile.print(",");
      logFile.println(cpm);
      logFile.close();
    } else {
      Serial.println("Error writing to geiger.csv");
    }

    counts = 0;  // Reset counter after logging
  }
}

