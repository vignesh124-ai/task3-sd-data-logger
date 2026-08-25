# Task 3: SD Card Data Logger (Temperature + Light)

Samples an LM35 temperature sensor and an LDR light sensor at a fixed interval, timestamps each reading (using a DS3231 RTC if available, or elapsed time otherwise), and appends the readings as CSV rows to `datalog.csv` on an SD card.

## Files
- **sd_temp_light_logger.ino** — Arduino sketch (SPI SD logging + optional I2C RTC + CSV output)
- **wiring_diagram_task3.png** — Circuit wiring diagram
- **Task3_README.docx** — Full write-up: objective, components, wiring tables, explanation, sampling rate & power considerations, and code
- **datalog_example.csv** — Example output showing the expected CSV format (replace with your real logged file, 100+ samples, before final submission)

## Hardware
- Arduino UNO
- SD card module (SPI)
- DS3231 RTC module (optional — for real-world timestamps)
- LM35 analog temperature sensor
- LDR + 10kΩ resistor (voltage divider)
- MicroSD card (FAT16/FAT32), breadboard, jumper wires

## Wiring Summary
| Module | Pin | Arduino UNO | Notes |
|---|---|---|---|
| SD Module | CS | D10 | Chip select |
| SD Module | MOSI | D11 | SPI data out |
| SD Module | MISO | D12 | SPI data in |
| SD Module | SCK | D13 | SPI clock |
| SD Module | VCC/GND | 5V/GND | Power |
| DS3231 RTC | SDA/SCL | A4/A5 | I2C (optional) |
| LM35 | OUT | A0 | Analog temp reading |
| LDR divider | Tap | A1 | Between LDR and 10kΩ to GND |

## Behavior
- Samples every 5 seconds by default (`SAMPLE_INTERVAL_MS`).
- Appends CSV rows as `timestamp,tempC,light_raw` to `datalog.csv`, flushing after every write so data survives a sudden power loss.
- Works with or without an RTC — set `USE_RTC` to `false` in the sketch if no DS3231 is connected.

See `Task3_README.docx` for full sampling-rate and power-consideration notes.
