/*
  Task 3: SD Card Data Logger (Temperature + Light, Timestamped)
  ------------------------------------------------------------------
  Hardware : Arduino UNO, SD card module (SPI), DS3231 RTC (I2C, optional),
             LM35 temperature sensor, LDR (light-dependent resistor)
  Software : SD.h, SPI.h, Wire.h, RTClib (optional - only if RTC is used)

  Wiring:
    SD card module (SPI bus - fixed on UNO):
      MOSI -> D11
      MISO -> D12
      SCK  -> D13
      CS   -> D10
      VCC  -> 5V
      GND  -> GND

    DS3231 RTC (optional, I2C):
      VCC -> 5V
      GND -> GND
      SDA -> A4
      SCL -> A5

    LM35 (temperature):
      VCC -> 5V
      GND -> GND
      OUT -> A0

    LDR (light, voltage divider):
      5V -> LDR -> A1 -> 10k resistor -> GND

  Behavior:
    - Samples temperature + light once every SAMPLE_INTERVAL_MS.
    - Timestamps each row using the RTC if present, otherwise falls back
      to millis()-based elapsed time.
    - Appends each reading as a CSV row to datalog.csv on the SD card,
      and mirrors it to Serial for live monitoring.
    - Writes a header row only if the file is new/empty.
    - Calls file.flush() after every write so data already on the card
      survives a power loss, at the cost of slightly higher write overhead.

  NOTE: If you don't have a DS3231, set USE_RTC to false below - the
  sketch will log elapsed milliseconds instead of a wall-clock timestamp.
*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>

#define USE_RTC true   // set to false if no DS3231 is connected

#if USE_RTC
  #include "RTClib.h"
  RTC_DS3231 rtc;
#endif

const int CHIP_SELECT = 10;       // SD module CS pin
const int TEMP_PIN = A0;          // LM35 analog pin
const int LIGHT_PIN = A1;         // LDR analog pin
const unsigned long SAMPLE_INTERVAL_MS = 5000; // 1 sample every 5 seconds

File logFile;
unsigned long lastSampleTime = 0;
unsigned long sampleCount = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println("Task 3: SD Card Data Logger");

  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("ERROR: SD card initialization failed. Check wiring/card.");
    while (1) { delay(1000); } // halt - nothing useful to do without SD
  }
  Serial.println("SD card initialized.");

  #if USE_RTC
    if (!rtc.begin()) {
      Serial.println("ERROR: RTC not found. Check wiring.");
      while (1) { delay(1000); }
    }
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, setting time to compile time.");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  #endif

  bool needsHeader = !SD.exists("datalog.csv");
  logFile = SD.open("datalog.csv", FILE_WRITE);
  if (logFile) {
    if (needsHeader) {
      logFile.println("timestamp,tempC,light_raw");
      logFile.flush();
    }
    logFile.close();
    Serial.println("Log file ready: datalog.csv");
  } else {
    Serial.println("ERROR: could not open datalog.csv");
  }

  Serial.println("timestamp,tempC,light_raw");
}

String getTimestamp() {
  #if USE_RTC
    DateTime now = rtc.now();
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
    return String(buf);
  #else
    return String(millis() / 1000) + "s"; // elapsed seconds since power-on
  #endif
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleTime < SAMPLE_INTERVAL_MS) return;
  lastSampleTime = now;

  int tempRaw = analogRead(TEMP_PIN);
  float tempC = (tempRaw * 5.0 / 1023.0) * 100.0; // LM35: 10mV/°C
  int lightRaw = analogRead(LIGHT_PIN);

  String timestamp = getTimestamp();
  String row = timestamp + "," + String(tempC, 2) + "," + String(lightRaw);

  logFile = SD.open("datalog.csv", FILE_WRITE);
  if (logFile) {
    logFile.println(row);
    logFile.flush(); // ensure this row survives an unexpected power loss
    logFile.close();
    sampleCount++;
  } else {
    Serial.println("ERROR: could not open datalog.csv for append");
  }

  Serial.println(row);

  if (sampleCount == 100) {
    Serial.println("100 samples logged. Logging will continue - "
                    "remove/pull the card once you have enough data.");
  }
}
