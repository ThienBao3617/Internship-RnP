 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.2
 *
 * @version Package Version: 3.1.2
*/

/*
© [2025] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/
// main.c
#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PIN6_bm     // PC6
#define BUTTON_PIN PIN7_bm  // PC7

// Global Variable
volatile uint8_t mode = 1;
volatile uint8_t led_state = 1; // 1: high, 0: low

int main(void) {
    // 1. C?u hình clock: 24MHz -> chia 6 = 4MHz
    CCP = 0xD8; // M? khóa ghi protected
    CLKCTRL.MCLKCTRLB = (0x01 << CLKCTRL_PDIV_gp) | CLKCTRL_PEN_bm;
    
    // 2. C?u hình GPIO
    VPORTC.DIR |= LED_PIN;     // PC6 output
    VPORTC.OUT &= ~LED_PIN;    // B?t LED (active-low)
    PORTC.PIN7CTRL |= PORT_PULLUPEN_bm; // PC7 input v?i pull-up
    
    // Bi?n cho nút nh?n
    uint8_t button_pressed = 0;
    uint16_t press_duration = 0;
    
    // Bi?n th?i gian cho LED
    uint32_t last_time = 0;
    uint16_t led_counter = 0;
    
    while (1) {
        // ??c tr?ng thái nút nh?n
        uint8_t button_current = (VPORTC.IN & BUTTON_PIN) ? 0 : 1; // 1: nh?n, 0: th?
        
        // X? lý nút nh?n
        if (button_current) {
            if (!button_pressed) {
                // Ghi nh?n l?n nh?n m?i
                button_pressed = 1;
                press_duration = 0;
            } else {
                // T?ng th?i gian nh?n (m?i l?n l?p ~10ms)
                if (press_duration < 65000) press_duration++;
            }
        } else {
            if (button_pressed) {
                // Nút v?a ???c th?
                if (press_duration > 300) { // > 3s
                    // Chuy?n ch? ??
                    mode = (mode % 3) + 1;
                    // Reset LED theo ch? ??
                    switch(mode) {
                        case 1: 
                            VPORTC.OUT &= ~LED_PIN; // B?t LED
                            led_state = 1;
                            break;
                        case 2: 
                            VPORTC.OUT |= LED_PIN;  // T?t LED
                            break;
                        case 3: 
                            VPORTC.OUT &= ~LED_PIN; // B?t LED
                            break;
                    }
                    led_counter = 0;
                } else {
                    // Nh?n ng?n: ch? x? lý ? ch? ?? 1
                    if (mode == 1) {
                        if (led_state) {
                            VPORTC.OUT |= LED_PIN;  // T?t LED
                            led_state = 0;
                        } else {
                            VPORTC.OUT &= ~LED_PIN; // B?t LED
                            led_state = 1;
                        }
                    }
                }
                button_pressed = 0;
            }
        }
        
        // X? lý LED theo ch? ??
        switch(mode) {
            case 1:
                // Không c?n làm gì thêm
                break;
                
            case 2: // Nh?p nháy 100ms
                if (led_counter >= 10) { // 50 * 10ms = 500ms
                    VPORTC.OUT ^= LED_PIN; // ??o LED
                    led_counter = 0;
                }
                break;
                
            case 3: // Sáng 100ms, t?t 900ms
                if (led_counter == 10) { // 100ms
                    VPORTC.OUT |= LED_PIN;   // T?t LED
                } else if (led_counter >= 100) { // 1000ms
                    VPORTC.OUT &= ~LED_PIN;  // B?t LED
                    led_counter = 0;
                }
                break;
        }
        
        // T?ng b? ??m th?i gian
        led_counter++;
        
        // Delay 10ms
        _delay_ms(10);
    }
}