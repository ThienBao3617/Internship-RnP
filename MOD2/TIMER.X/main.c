#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 4000000UL // 4MHz clock

// Mode definitions
#define MODE_1 1
#define MODE_2 2
#define MODE_3 3

// Button timing constants
#define DEBOUNCE_TIME 20
#define LONG_PRESS_TIME 3000

// LED blink timings
#define MODE2_PERIOD 50    // 50ms on/off
#define MODE3_ON_TIME 100  // 100ms on
#define MODE3_OFF_TIME 900 // 900ms off

volatile uint8_t current_mode = MODE_1; // Current operating mode (1, 2, or 3)
volatile uint16_t button_counter = 0; // Button press duration counter (in ms)
volatile uint16_t blink_counter = 0; 
volatile uint8_t button_pressed = 0; // Flag indicating button is pressed (debounced)
volatile uint8_t button_debounced = 0; // Flag indicating complete button event (button released)

// Timer setup for 1ms interrupts
void timer_init(void) {
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable overflow interrupt
    TCA0.SINGLE.PER = 3999;                 // 4MHz / 1 / 4000 = 1000Hz (1ms)
    TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // Enable timer, prescaler = 1
}

// Button initialization
void button_init(void) {
    PORTC.DIRCLR = PIN7_bm;    // Set PC7 as input (button)
    PORTC.PIN7CTRL = PORT_PULLUPEN_bm; // Enable pull-up resistor
}

// LED initialization
void led_init(void) {
    PORTC.DIRSET = PIN6_bm;    // Set PC6 as output (LED)
    PORTC.OUTCLR = PIN6_bm;    // LED on initially (active low)
}

ISR(TCA0_OVF_vect) {
    static uint8_t debounce_counter = 0;
    
    // Clear interrupt flag
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
    
    // Button state machine
    uint8_t button_state = ~PORTC.IN & PIN7_bm; // Read button state
    
    if (button_state) { // Button pressed
        if (debounce_counter < DEBOUNCE_TIME) {
            debounce_counter++;
        } 
        else {
            button_pressed = 1;
            button_counter++;
        }
    } 
    else { // Button released
        if (debounce_counter > 0) {
            debounce_counter--;
        } 
        else {
            if (button_pressed) {
                button_pressed = 0;
                button_debounced = 1;
            }
        }
    }
    
    // Mode-specific LED handling
    switch (current_mode) {
        case MODE_2:
            blink_counter++;
            if (blink_counter >= MODE2_PERIOD) {
                PORTC.OUTTGL = PIN6_bm; // Toggle LED
                blink_counter = 0;
            }
            break;
            
        case MODE_3:
            blink_counter++;
            if (PORTC.OUT & PIN6_bm) { // LED currently off
                if (blink_counter >= MODE3_OFF_TIME) {
                    PORTC.OUTCLR = PIN6_bm; // Turn on
                    blink_counter = 0;
                }
            } 
            else { // LED currently on
                if (blink_counter >= MODE3_ON_TIME) {
                    PORTC.OUTSET = PIN6_bm; // Turn off
                    blink_counter = 0;
                }
            }
            break;
    }
}

int main(void) {
    // Initialize peripherals
    led_init();
    button_init();
    timer_init();
    
    // Enable global interrupts
    sei();
    
    while (1) {
        if (button_debounced) {
            button_debounced = 0;
            
            switch (current_mode) {
                case MODE_1:
                    // Short press: toggle LED
                    if (button_counter < LONG_PRESS_TIME) {
                        PORTC.OUTTGL = PIN6_bm;
                    }
                    // Long press: mode change
                    else if (button_counter >= LONG_PRESS_TIME) {
                        current_mode = MODE_2;
                        blink_counter = 0;
                        PORTC.OUTSET = PIN6_bm; // Start with LED off
                    }
                    break;
                    
                case MODE_2:
                    // Long press only
                    if (button_counter >= LONG_PRESS_TIME) {
                        current_mode = MODE_3;
                        blink_counter = 0;
                        PORTC.OUTCLR = PIN6_bm; // Start with LED on
                    }
                    break;
                    
                case MODE_3:
                    // Long press only
                    if (button_counter >= LONG_PRESS_TIME) {
                        current_mode = MODE_1;
                        PORTC.OUTCLR = PIN6_bm; // LED on
                    }
                    break;
            }
            button_counter = 0;
        }
    }
}