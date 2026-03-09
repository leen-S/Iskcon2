# Dhanu – Devotional Embedded Device

A portable devotional device built using ESP32, designed to assist with daily chanting, streak tracking, and time awareness.  
The device features an animated home screen, a chant counter, streak tracking, and a real-time clock, all displayed on an OLED screen.

This project combines embedded systems, UI design, and spiritual discipline tools into a single handheld device.

---

# Features

## Animated Home Screen

- Krishna silhouette animation
- Smooth 50 FPS rendering
- Optimized for SSD1306 OLED display

## Chant Counter

- Counts from 1 to 108
- Displays "Hari Bol" when the round is complete
- Simple button interaction

## Streak Tracker

- Tracks daily chanting streak
- Uses RTC for accurate date tracking
- Stores data persistently

## Time Applet

Displays real-time clock from DS3231.

Example display:

```
Time
18:10:34

9th March 2026
```

## Custom Boot Sequence

On startup the device displays:

```
Happy B'day
Dhanu

By Leen
```

Followed by a butterfly animation before entering the home screen.

---

# Architecture

The firmware uses a state-machine based UI system.

```
BOOT
  ↓
HOME
  ↓
MENU
 ├── Chant
 ├── Streak
 └── Time
```

The system is designed to be:

- Non-blocking
- Modular
- Expandable

The animation engine runs at 50 FPS while UI interactions remain responsive.

---

# Hardware

## Microcontroller

ESP32 DevKit V1

## Display

SSD1306 OLED (128x64)

## Real Time Clock

DS3231 RTC module

## Input

Push button connected to GPIO0.

---

# Wiring

## OLED Display

| OLED | ESP32 |
|-----|------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

## RTC DS3231

| DS3231 | ESP32 |
|------|------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

Both devices share the same I2C bus.

---

# Software

Built using:

- PlatformIO
- Arduino Framework
- RTClib
- Adafruit SSD1306
- Adafruit GFX

Example platformio.ini dependencies:

```
lib_deps =
  adafruit/Adafruit SSD1306
  adafruit/Adafruit GFX Library
  adafruit/RTClib
```

---

# Getting Started

## Clone the repository

```
git clone https://github.com/leen-S/Iskcon2.git
```

## Open in VSCode with PlatformIO

Open the project folder inside Visual Studio Code.

## Build the project

```
pio run
```

## Upload firmware

```
pio run -t upload
```

## Open Serial Monitor

```
pio device monitor
```

---

# Testing the RTC

The RTC time can be verified using the serial monitor.

Example output:

```
Time: 18:10:01   Date: 9/3/2026
Time: 18:10:02   Date: 9/3/2026
Time: 18:10:03   Date: 9/3/2026
```

---

# Future Improvements

Planned features include:

- Buzzer feedback after completing 108 chants
- MP3 player for bhajans or mantras
- Longest streak tracking
- Battery powered portable design
- Bluetooth audio support

---

# Author

Leen

Embedded systems enthusiast building devotional hardware tools using ESP32.

---

# License

MIT License
