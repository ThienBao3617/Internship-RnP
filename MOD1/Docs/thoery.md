# LED Handling with Button through GPIO Control on AVR128DA48 

## Table of Contents

1. [Introduction](#introduction)
2. [MCU Modules Used](#mcu-modules-used)
3. [Theory and Implementation](#theory-and-implementation)

---

## Introduction

This document explains how to control an LED and read a push-button on the AVR128DA48 in three modes:

* **Mode 1:** LED stays on by default; a single press toggles the LED state; press button and hold about 3s then release to switch to **mode 2**.
* **Mode 2:** LED blinks with a 100 ms period; then hold for 3s and release to switch to **mode 3**.
* **Mode 3:** LED on for 100 ms, off for 900 ms; continue hold for 3s then release to switch back to **mode 1**.

Focus areas: clock setup, GPIO configuration, and push-button handling (debouncing).

---

## MCU Modules Used

| Module              | Function                           | Datasheet Reference |
| ------------------- | ---------------------------------- | ------------------- |
| Clock System        | Configure system clock (Prescaler) | Chapter 11 - Section 11.2.1          |
| GPIO                | Configure input/output             | Chapter 17 - Section 17.5          |
| Protected I/O (CCP) | Protect critical register writes   | Chapter 11 - Section 11.3.7         |

---

## Theory and Implementation

### 1. Clock Setup

**Theory:**

* The AVR128DA48 features an internal 24 MHz oscillator.
* Clock prescalers allow dividing the system clock frequency.
* CCP (Configuration Change Protection) prevents accidental writes to critical registers by requiring a specific unlock sequence.

**Code:**

```c
CCP = 0xD8;  // Unlock protected registers
CLKCTRL.MCLKCTRLB = (0x01 << CLKCTRL_PDIV_gp) | CLKCTRL_PEN_bm;
```

* **CCP Unlock Sequence (0xD8):**
    - Writing `0xD8` (binary `11011000`) temporarily disables protection

    - Must be followed immediately by the protected register write

* **Clock Prescaler Configuration:**

    - `CLKCTRL_PEN_bm`: Enables prescaler (bit 0 = 1)

    - `(0x08 << CLKCTRL_PDIV_gp)`: Sets prescaler division factor:

        - `CLKCTRL_PDIV_gp` = bit position 1 (group position)

        - `0x08` shifted left 1 bit -> value `0x10` (binary `00010000`), then `plus 0x01 = 0x11` => Look at `PDIV[3:0]` index = 8 -> apply a `divide‑by‑6(DIV6)` to the 24 MHz oscillator
        - Result: system clock `(CLK_PER) = 24 MHz ÷ 6 = 4 MHz`

**Why 4 Mhz?** 
- Optimal balance between processing capability and power efficiency for this application.
---

### 2. GPIO Configuration

**Theory:**

* Each pin can be independently input or output.
* Inputs read logic levels and support internal pull-ups.
* Outputs drive high/low levels.

**Code:**

```c
// Config LED on PC6 (active-low)
VPORTC.DIR  |= PIN6_bm;
VPORTC.OUT  &= ~PIN6_bm;

// Config Button on PC7 (active-low)
PORTC.PIN7CTRL |= PORT_PULLUPEN_bm;
```

* PC6 as output; driving low lights the LED.
* PC7 as input with internal pull-up.

---

### 3. Button Handling and Timing

**Theory:**

* Mechanical switches bounce (\~5–50 ms).
* Software debouncing uses a delay (10 ms).
* Counting loops distinguishes short vs. long presses.

**Code:**

```c
// Global variable
uint8_t  button_pressed  = 0;     // flag for button state
uint16_t press_duration   = 0;    // time for pressing (10ms unit)

// Main loop
uint8_t button_current = (VPORTC.IN & BUTTON_PIN) ? 0 : 1;

if (button_current) {
    if (!button_pressed) {
        button_pressed  = 1;
        press_duration  = 0;
    } else {
        press_duration++;
    }
}
// handle when release button
_delay_ms(10);  // create delay as 10ms
```

* **Read button status:**

  * VPORTC.IN: Register to read the status of Port C pins
  * (VPORTC.IN & BUTTON_PIN) ? 0 : 1:

    * If PC7 pin is low (button pressed) -> return 1
    * Otherwise -> return 0
* **Button pressed detection:**

  * When first button pressed detected (!button_pressed):

    * Set button_pressed flag = 1
    * Reset timer press_duration = 0
  * When button is held:

    * Increase press_duration every 10ms
* **Button released detection:**

  * When button is released (button_pressed == 1 and button_current == 0):

    * If press_duration > 300 (300 × 10ms = 3s): Handle long press
    * Otherwise: Handle short press
* **Debouncing:**

  * 10ms time interval is large enough to ignore key vibration
  * Only change state when signal is stable after 10ms

**Why choose 10ms?**
  * Typical key vibration time: <50ms
  * 10ms ensures skipping of vibration phase

---

## Summary of Modules

| Module        | Role                         | Setting    | Registers Used         |
| ------------- | ---------------------------- | ---------- | ---------------------- |
| Clock System  | 4 MHz system clock           | DIV6       | MCLKCTRLB              |
| GPIO - LED    | PC6 LED control (active-low) | OUTPUT     | VPORTC.DIR, VPORTC.OUT |
| GPIO - Button | PC7 push-button with pull-up | INPUT      | PIN7CTRL, VPORTC.IN    |
| Debounce      | Bounce filtering & timing    | 10 ms/loop | `_delay_ms()`          |
