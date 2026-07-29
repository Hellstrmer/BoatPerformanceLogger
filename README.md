# BoatPerformanceLogger

Telemetry and instrumentation system for a Hydrolift T18 with a Yamaha Autolube 150 hp outboard. Built under the **Plutoid Engineering** brand.

The system reads sensors at the engine, streams live data to a display over ESP-NOW, and (in progress) logs everything to SD for later analysis — the goal being to work out which trim, jackplate height and propeller combination actually runs fastest.

## Architecture

Two ESP32 nodes talking over ESP-NOW:

| Node | Board | Role |
|------|-------|------|
| **BoatPerformanceSensor** | ESP32 | Reads all sensors at the engine, pitch and GPS, logs to SD, serves the dashboard over its own AP |
| **BoatPerformanceDisplay** | ESP32-C6 | Round GC9A01A display + web dashboard at the helm | *Will be removed from project*
| **BoatPerformanceDash** | Raspberry Pi *(Unknow model yet)* | 7" HDMI Screen + web dashboard accessable from Mobile |


## Sensors

| Signal | Method | Status |
|--------|--------|--------|
| Jackplate height | Magnetic quadrature encoder (0.005 mm/pulse) | Done |
| Engine RPM | Tach pulse from rectifier → optocoupler → ESP32 PCNT | In progress |
| GPS speed / position | ATGM336H module | Planned |
| Trim angle | Resistive sender via ADC | Planned |
| Pitch / roll / G | BNO085 IMU (on-chip fusion) | Planned |
| Propeller slip | Calculated from RPM, gear ratio, prop pitch and GPS speed | Planned |
| Fuel flow | Derived from an RPM→fuel curve built from tank top-ups | Planned |

## Propeller slip

Slip is derived, not measured:

```
slip % = (1 − V_actual / V_theoretical) × 100
V_theoretical (kn) ≈ (RPM × pitch_in) / (gear_ratio × 1215)
```

`V_actual` comes from GPS. On the dashboard the slip is drawn as the gap between actual RPM and the RPM the boat *would* be turning at zero slip.

## Repository layout

```
BoatPerformanceLogger/
├── BoatPerformanceDash/    # 7" Dash (PlatformIO project)
├── BoatPerformanceSensor/    # engine node (PlatformIO project)
├── BoatPerformanceDisplay/   # heWill be removed
├── .gitignore
└── README.md
```

Each subfolder is its own PlatformIO project — open the relevant one in VS Code to work on that node.

## Hardware

- 1x ESP32 (sensor node classic ESP32, display node ESP32-C6)
- 1 Raspberry Pi (Display)
- GC9A01A 240×240 round TFT (SPI) *Soon Removed*
- Magnetic encoder for jackplate position
- PC817 optocoupler + zener clamp for the tach signal
- *(more to come as sensors are added)*

## Build

Built with [PlatformIO](https://platformio.org/) using the
[pioarduino](https://github.com/pioarduino/platform-espressif32) fork for
ESP32-C6 support.

## Rig constants

Several values must be set for your specific engine and prop before slip and RPM read correctly:

| Constant | Where | Note |
|----------|-------|------|
| `PULSES_PER_REV` | `rpm.h` | 6 for a 12-pole Yamaha V-block — verify against the factory tacho |
| `GEAR` | dashboard / sensor | Gearbox ratio, verify for the Autolube 150 |
| `PITCH_IN` | dashboard / sensor | Propeller pitch in inches |
| `REDLINE` | dashboard | Check the engine manual |

## License

TBD

---

*Plutoid Engineering*