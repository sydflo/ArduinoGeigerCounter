#include <LiquidCrystal.h>
#include <SD.h>

#define LOG_PERIOD 15000        // Log interval (ms)
#define MAX_PERIOD 60000        // Used for CPM multiplier
#define PULSE_PIN 2             // Geiger signal
#define SD_CS_PIN 4             // SD card CS

// LCD pinout: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(6, 5, 3, A0, A1, A2);

unsigned long counts = 0;
unsigned long cpm = 0;
unsigned long previousMillis = 0;
unsigned int multiplier = MAX_PERIOD / LOG_PERIOD;

File logFile;
const char* location = "Smelter Park";  // Update as needed

void tube_impulse() {
  counts++;
}

void setup() {
  Serial.begin(9600);

  // Geiger setup
  pinMode(PULSE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), tube_impulse, FALLING);

  // LCD setup
  lcd.begin(16, 2);
  lcd.print("Initializing...");
  delay(1000);
  lcd.clear();

  // SD card setup
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card failed!");
    lcd.print("SD init failed!");
    while (1);
  }
  Serial.println("SD ready.");
  lcd.print("SD ready.");
  delay(1000);
  lcd.clear();

  logFile = SD.open("geiger.csv", FILE_WRITE);
  if (logFile) {
    logFile.println("Time(s),CPM,Location");
    logFile.close();
    Serial.println("Logging started.");
  } else {
    Serial.println("File create failed.");
    lcd.print("File error!");
    while (1);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= LOG_PERIOD) {
    previousMillis = currentMillis;
    cpm = counts * multiplier;
    float timeInSeconds = currentMillis / 1000.0;

    // Print to Serial
    Serial.print("CPM: ");
    Serial.print(cpm);
    Serial.print(" at ");
    Serial.print(timeInSeconds, 1);
    Serial.println("s");

    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CPM: ");
    lcd.print(cpm);

    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    lcd.print(timeInSeconds, 1);
    lcd.print("s");

    // Write to SD card
    logFile = SD.open("geiger.csv", FILE_WRITE);
    if (logFile) {
      logFile.print(timeInSeconds, 1);
      logFile.print(",");
      logFile.print(cpm);
      logFile.print(",\"");
      logFile.print(location);
      logFile.println("\"");
      logFile.close();
    } else {
      Serial.println("SD write error.");
    }

    counts = 0;
  }
}
