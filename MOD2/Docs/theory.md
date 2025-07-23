# LED Handling with Button through Timer/Interrupt on AVR128DA48 
## Table of Contents

1. [Overview](#overview)
2. [Hardware](#hardware)
3. [MCU Modules Used](#mcu-modules-used)
4. [Theory and Implementation](#theory-and-implementation)
5. [Summary](#Summary)
---

## 1. Overview
This application demonstrates the use of a timer (TCA0) and interrupts to control an LED in 3 different modes based on button press duration:

- **Mode 1:** LED on. Short press toggles LED. Press and hold for 3s to enter **Mode 2**.

- **Mode 2:** LED blinks with 100ms period. Press and hold for 3s to enter **Mode 3**.

- **Mode 3:** LED on for 100ms, off for 900ms. Press and hold for 3s to return to **Mode 1**.
---

## 2. Hardware
- MCU: AVR128DA48 (Curiosity Nano)

- LED: PC6 (Active Low -> 0: ON, 1: OFF)

- Button: PC7 (Pull-up, pressed = 0)
---

## 3. MCU Modules Used

| Module              | Function                           | Datasheet Reference |
| ------------------- | ---------------------------------- | ------------------- |
| Timer        | TCA | Chapter 21 - Section 21.5          |
| GPIO                | Configure input/output             | Chapter 17 - Section 17.5          |
---

## 4. Theory and Implementation
### 4.1. Timer TCA0 (Timer Counter Type A)
- Configured for 1ms interrupt (1kHz) from 4MHz clock source
- PERIOD value calculation:
  ```
  T = (PERIOD + 1) * (1 / F_CPU) * prescaler
  0.001 = (PERIOD + 1) * (1/4000000) * 1
  => PERIOD = 3999
  ```
- Configuration registers:
  - `TCA0.SINGLE.PER = 3999`: Set maximum count value
  - `TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm`: Enable overflow interrupt
  - `TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm`: Enable timer, prescaler=1
### 4.2. Button Handling
- Software debounce mechanism (20ms) in timer interrupt

- Count button hold duration (button_counter increments every 1ms)

- On button release:

    - If button_counter < 3000: Short press (Mode 1: toggle LED)

    - If button_counter >= 3000: Long press (change mode)
### 4.3. LED Control

- **Mode 1:** LED on by default. Short press toggles LED.

- **Mode 2:** In timer interrupt, toggle LED every 50ms -> 100ms period.

- **Mode 3:**

    - LED on: count 100ms -> turn off LED

    - LED off: count 900ms -> turn on LED
### 4.4. Mode Switching

- current_mode variable stores current operating mode

- On long press detection (3s), switch mode and reset LED state

## 5. Summary
- The application efficiently uses 1ms timer interrupts for precise timing control

- Direct register access approach optimizes code and provides hardware-level control

- Clear state machine structure enables easy expansion
