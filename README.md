# AVR128DA48 on Curiosity Nano Explorer

This document provides a concise overview of my internship tasks. Detailed theory, code, and results for each module will be maintained in separate files within their respective folders.

## Goals

* Familiarize with the Curiosity Nano Explorer hardware and AVR128DA48 MCU
* Establish build and debugging workflows (MPLAB X, VSCode)
* Apply MCU features to onboard modules in incremental steps

## High-Level Workflow

1. **Setup & Validation**

   * Configure project environment and toolchains
   * Verify power rails, clock sources, and debug interface

2. **MCU Configuration**

   * Initialize system clock and protected registers (CCP)
   * Configure GPIO ports and basic I/O

3. **Module Development**
   Implement and test each module in this order:

   * **GPIO**: LED control (toggle/blink), button debouncing
   * **Timer/PWM**: Periodic interrupts, LED brightness control
   * **UART**: Serial communication for logging and commands
   * **ADC**: Read analog sensors (e.g. potentiometer)
   * **I2C / SPI**: Interface with EEPROMs and sensors
   * **USB / Touch**: USB bridge and touch button handling

4. **Documentation & Verification**

   * For each module: theory, algorithm flowchart, annotated code, and proof of execution (photos or logs)

## Repository Structure

* `DAYx/ModuleName/` – Source code and `theory.md` for each module
* `Docs/` – Theory, flowcharts, schematics, and images.
* `.gitignore` – Excluded files


