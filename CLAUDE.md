# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

**Pinballed** is an LED controller that syncs to pinball machine lights/colors.

## Project Layout

```
pinballed/
├── sense/       # Raspberry Pi — camera-based color sensing, and potentially sound sensing
└── ctrl/        # Communication and LED controller
    ├── tx/      # ESP32 transmitter (receives from Pi, sends via ESP-NOW)
    ├── led/     # ESP32 receiver (receives ESP-NOW, drives LEDs)
    └── shared/  # Shared packet formats, protocol constants (used by both tx and rx)
```

## Build System

- `tx` and `led` are PlatformIO projects
- `sense` is based on OpenCV and runs on Raspberry Pi

## Hardware Architecture

- **Raspberry Pi** with a camera module — captures pinball light/color state
- **ESP32 (transmitter)** — connected to the Raspberry Pi, sends instructions over ESP-NOW
- **ESP32 (receiver)** — receives instructions over ESP-NOW and drives the LEDs
  - **LEDs** - A strip of 60 WS2812B LEDs