 /*
 * File:   main.c
 * Brief: GPIO & Button task for AVR128DA48 Curiosity Nano switching of between 3 modes of LED 
 * Author: skytold
 *
 * Created on June 25, 2025
 */

#include <avr/io.h>
#include <util/delay.h>

// Pin definitions for Curiosity Nano board
#define LED_PIN PIN6_bm     // PC6 (LED0 active-low)
#define BUTTON_PIN PIN7_bm  // PC7 (SW0 active-low, with pull-up)

// Global variables
volatile uint8_t mode = 1;      // Current mode (1,2,3)
volatile uint8_t led_state = 1; // LED state in mode 1 (1: ON, 0: OFF)

int main(void) {
    // SECTION 1: CLOCK CONFIGURATION
    // -------------------------------
    // Configure system clock: 24MHz internal oscillator divided by 6 to get 4MHz
    CCP = 0xD8;                 // Unlock protected write
    CLKCTRL.MCLKCTRLB = (0x08 << CLKCTRL_PDIV_gp) | CLKCTRL_PEN_bm; // Prescaler division by 6
    
    // SECTION 2: GPIO CONFIGURATION
    // -----------------------------
    // LED (PC6) as output, initially ON (active-low: 0=ON, 1=OFF)
    VPORTC.DIR |= LED_PIN;      // Set PC6 as output 
    VPORTC.OUT &= ~LED_PIN;     // Set PC6 low (LED ON) 
    
    // Button (PC7) as input with pull-up enabled
    PORTC.PIN7CTRL |= PORT_PULLUPEN_bm; // Enable pull-up on PC7 
    
    // Button state variables
    uint8_t button_pressed = 0;     // Flag for button press
    uint16_t press_duration = 0;    // Duration of button press (in 10ms units)
    
    // Timing variables for LED control
    uint16_t led_counter = 0;       // Counter for LED timing (in 10ms units)
    
    // Main loop
    while (1) {
        // SECTION 3: BUTTON INPUT HANDLING
        // --------------------------------
        // Read button state: 1 if pressed (active-low), 0 if released
        uint8_t button_current = (VPORTC.IN & BUTTON_PIN) ? 0 : 1;
        
        // Button press/release detection
        if (button_current) {       // Button is pressed
            if (!button_pressed) {
                // New press: record start time
                button_pressed = 1;
                press_duration = 0;
            } 
            else {
                // Button still pressed: increment duration
                if (press_duration < 65000) press_duration++;
            }
        } 
        else {                    // Button is released
            if (button_pressed) {
                // Button was just released
                if (press_duration > 300) { // Pressed for more than 300*10ms = 3s
                    // Change mode: 1->2, 2->3, 3->1
                    mode = (mode % 3) + 1;
                    
                    // Reset LED according to new mode
                    switch(mode) {
                        case 1: 
                            VPORTC.OUT &= ~LED_PIN; // LED ON (active-low: set low)
                            led_state = 1;          // Update state
                            break;
                        case 2: 
                            VPORTC.OUT |= LED_PIN;  // LED OFF (set high)
                            break;
                        case 3: 
                            VPORTC.OUT &= ~LED_PIN; // LED ON (set low)
                            break;
                    }
                    led_counter = 0; // Reset LED timing counter
                } 
                else { // Short press (<= 3s)
                    // Only toggle LED in mode 1
                    if (mode == 1) {
                        if (led_state) {
                            VPORTC.OUT |= LED_PIN;   // Turn LED OFF
                            led_state = 0;
                        } 
                        else {
                            VPORTC.OUT &= ~LED_PIN;  // Turn LED ON
                            led_state = 1;
                        }
                    }
                }
                button_pressed = 0; // Reset button state
            }
        }
        
        // SECTION 4: LED OUTPUT CONTROL
        // -----------------------------
        // Handle LED behavior based on current mode
        switch(mode) {
            case 1:
                // No periodic action needed (handled by button press)
                break;
                
            case 2: // Blink LED every 100ms (10 cycles * 10ms = 100ms)
                if (led_counter >= 10) {
                    VPORTC.OUT ^= LED_PIN; // Toggle LED state
                    led_counter = 0;       // Reset counter
                }
                break;
                
            case 3: // LED ON for 100ms, OFF for 900ms (total 1000ms cycle)
                if (led_counter == 10) {       // After 100ms
                    VPORTC.OUT |= LED_PIN;     // Turn OFF LED
                } 
                else if (led_counter >= 100) { // After 1000ms
                    VPORTC.OUT &= ~LED_PIN;    // Turn ON LED
                    led_counter = 0;           // Reset counter
                }
                break;
        }
        
        // SECTION 5: TIMING MANAGEMENT
        // ----------------------------
        led_counter++;  // Increment LED timing counter
        _delay_ms(10);  // 10ms delay per loop iteration
    }
}