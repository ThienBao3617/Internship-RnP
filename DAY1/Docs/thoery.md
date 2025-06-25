# GPIO and Button Control on AVR128DA48

## Table of Contents
1. [Introduction](#introduction)
2. [MCU Modules Used](#mcu-modules-used)
3. [Theory and Implementation](#theory-and-implementation)
4. [Flowchart](#flowchart)
5. [Code Explanation](#code-explanation)
6. [Hardware Setup](#hardware-setup)
7. [Results](#results)

---

## Introduction
This document details the implementation of GPIO and button control on the AVR128DA48 microcontroller, featuring three operational modes:
- Mode 1: Default LED ON, toggle on button click
- Mode 2: LED blinking at 100ms period
- Mode 3: LED ON for 100ms, OFF for 900ms

---

## MCU Modules Used
| Module | Functionality | Datasheet Reference |
|--------|---------------|---------------------|
| Clock System | System clock configuration | Section 14 |
| GPIO | Digital input/output control | Section 16 |
| Protected I/O | Configuration change protection | Section 8.3 |

---

## Theory and Implementation

### 1. Clock System
**Theory**:
- The AVR128DA48 has an internal 24MHz oscillator
- Clock prescalers allow dividing the system clock
- Protected I/O mechanism (CCP) is required to modify critical registers

**Implementation**:
```c
CCP = 0xD8;  // Unlock protected write
CLKCTRL.MCLKCTRLB = (0x01 << CLKCTRL_PDIV_gp) | CLKCTRL_PEN_bm;
```
- Divides 24MHz clock by 6 → 4MHz system clock

- Uses Configuration Change Protection (CCP) mechanism
- Reference: **MCLKCTRLB** Register (datasheet chapter 11)

### 2. GPIO (General Purpose Input/Output)

**Theory**:

- Each pin can be configured as input or output

- Input pins can enable internal pull-up resistors

- Output pins can drive high/low logic levels

- Memory-mapped registers control pin behavior

**Implementation**:
```c

// LED (PC6) output configuration
VPORTC.DIR |= PIN6_bm;       // Set as output
VPORTC.OUT &= ~PIN6_bm;      // Set low (LED ON)

// Button (PC7) input configuration
PORTC.PIN7CTRL |= PORT_PULLUPEN_bm;  // Enable pull-up
```
- LED uses active-low configuration

- Button uses internal pull-up resistor

- Reference: **Register Description** (datasheet chapter 11)

### 3. Button Debouncing and Timing

**Theory**:

- Mechanical switches exhibit contact bounce (5-50ms)

- Time-based debouncing is more reliable than hardware

- Press duration measurement requires consistent timing

**Implementation**:
```c

_delay_ms(10);  // Consistent 10ms timing

if (button_current) {
    press_duration++;
}
```
- 10ms timing loop provides consistent timebase

- Press duration measured in 10ms increments

- 300 counts = 3s press detection
