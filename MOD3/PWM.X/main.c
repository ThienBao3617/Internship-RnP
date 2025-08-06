 /*
 * File:   main.c
 * Brief: ADC & PWM task for AVR128DA48 Curiosity Nano by adjusting potentiometer for LED's brightness and plot graphs onto Data Visualizer through UART
 * Author: skytold
 */

#define PERIOD_EXAMPLE_VALUE  0x0FF  // PWM period (8-bit = 255)
#define F_CPU 4000000UL               // Clock speed of MCU: 4MHz
#define BAUD_RATE 9600               // UART baud rate for serial output

#include <avr/io.h>
#include <util/delay.h>

// Function Declarations
void PORT_init(void);
void TCA1_init(void);
void ADC0_init(void);
uint16_t ADC0_read(void);
void USART0_init(void);
void USART0_sendChar(char c);
void USART0_sendString(const char* str);
void USART0_sendInt(uint16_t value);

// I/O Pin Configurations
void PORT_init(void) {
    PORTC.DIRSET |= PIN6_bm; // Set PC6 as output (for PWM signal to onboard LED)
    PORTA.DIRSET |= PIN0_bm; // Set PA0 as output (TX pin for UART)
}

// Timer/Counter A1 Initialization (PWM Setup) 
void TCA1_init(void) {
    PORTMUX.TCAROUTEA = PORTMUX_TCA1_PORTC_gc; // Route TCA1 waveform output to PORTC (PC6)

    TCA1.SINGLE.CTRLB = TCA_SINGLE_CMP2EN_bm              // Enable Compare Channel 2 (WO2 = PC6)
                      | TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // Use single-slope PWM mode

    TCA1.SINGLE.PER = PERIOD_EXAMPLE_VALUE;  // Set PWM period (defines frequency)
    TCA1.SINGLE.CMP2 = 0;                   // Initial duty cycle = 0 (LED off)

    TCA1.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc // Use system clock directly
                      | TCA_SINGLE_ENABLE_bm;     // Start the timer
}

// ADC Setup (for reading analog voltage from potentiometer) 
void ADC0_init(void) {
    VREF.ADC0REF = VREF_REFSEL_VDD_gc;               // Use VDD (5V) as reference voltage
    ADC0.CTRLC = ADC_PRESC_DIV16_gc;                 // Set ADC clock prescaler: 4MHz / 16 = 250kHz
    ADC0.CTRLA = ADC_RESSEL_10BIT_gc | ADC_ENABLE_bm;// Enable ADC, 10-bit resolution (0 - 1023)
    ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;                // Select ADC channel AIN0 (PD0)
}

// Read ADC value (10-bit, blocking call) 
uint16_t ADC0_read(void) {
    ADC0.COMMAND = ADC_STCONV_bm;                    // Start conversion
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));        // Wait until result is ready
    ADC0.INTFLAGS = ADC_RESRDY_bm;                   // Clear flag
    return ADC0.RES;                                 // Return 10-bit result
}

// UART Setup 
void USART0_init(void) {
    // Calculate baud register value using formula from datasheet
    uint16_t baud = (uint16_t)(F_CPU * 64 / (16 * (float)BAUD_RATE) + 0.5);
    USART0.BAUD = baud;              // Set baud rate
    USART0.CTRLB = USART_TXEN_bm;    // Enable transmitter only
}

// Send a single character over UART 
void USART0_sendChar(char c) {
    while (!(USART0.STATUS & USART_DREIF_bm)); // Wait for data register to be empty
    USART0.TXDATAL = c;                        // Load data to transmit register
}

// Send a null-terminated string over UART 
void USART0_sendString(const char* str) {
    while (*str) {
        USART0_sendChar(*str++); // Send each character one by one
    }
}

// Convert and send an integer value over UART 
void USART0_sendInt(uint16_t value) {
    char buffer[6];                              // Enough to hold 5-digit number + null terminator
    char *ptr = buffer + sizeof(buffer) - 1;
    *ptr = '\0';                                // Null-terminate the string

    do {
        *--ptr = '0' + (value % 10);             // Convert last digit to ASCII
        value /= 10;
    } while (value);

    USART0_sendString(ptr);                      // Send the converted string
}

int main(void) {
    PORT_init();       // Configure I/O pins
    ADC0_init();       // Initialize ADC
    TCA1_init();       // Initialize Timer/Counter A1 for PWM
    USART0_init();     // Initialize UART

    USART0_sendString("ADC,PWM\n"); // Header row for MPLAB Data Visualizer (CSV format)

    while (1) {
        uint16_t adc_value = ADC0_read();         // Read analog value (0 - 1023)
        uint8_t pwm_value = 255 - (adc_value >> 2); // Scale ADC to 8-bit and invert for LED brightness

        TCA1.SINGLE.CMP2 = pwm_value;             // Update PWM duty cycle

        USART0_sendInt(adc_value);                // Print ADC value
        USART0_sendChar(',');
        USART0_sendInt(255 - pwm_value);          // Print LED brightness
        USART0_sendChar('\n');

        _delay_ms(50);                            // Small delay to control update rate
    }
}
