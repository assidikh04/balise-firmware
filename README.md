# balise-firmware

Water-level beacon prototype (ESP32 + HC-SR04 ultrasonic sensor), built for the SENSAT hackathon ("From satellite to solution"). Part of the [mbennscan](https://github.com/TechwithXalifa/mbennscan) project.

▶️ [Simulate it on Wokwi](https://wokwi.com/projects/478161781301836801)

## How it works
- Takes 9 distance measurements per cycle and discards out-of-range values
- Requires at least 5 valid readings, then uses the **median** (robust to noisy echoes)
- Converts distance to water level (%) from a 40 cm reference height
- Classifies the state: `NORMAL`, `VIGILANCE` (≥ 50 %), `ALERTE` (≥ 80 %)
- Reports the margin before the alert threshold and a reliability score (valid readings / total)
- Diagnoses failures (no echo, object too close, nothing detected)
- Output over serial, as readable text or JSON (`FORMAT_JSON` in `sketch.ino`)

## Wiring
| HC-SR04 | ESP32 |
|---------|-------|
| VCC | 5V |
| GND | GND |
| TRIG | GPIO 5 |
| ECHO | GPIO 18 |

## Example output
`18.4 cm | [#####.....] 57% | VIGILANCE | alerte dans 8.8 cm`

## Files
- `sketch.ino`: firmware
- `diagram.json`: Wokwi circuit

## Next steps
Send the JSON output to a server (HTTP) for the map and alerts.
