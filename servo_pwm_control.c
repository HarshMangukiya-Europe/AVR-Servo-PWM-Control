#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Global flag to control flashing of LED on D13
volatile uint8_t flash_enable = 0;

int main(void) {
    // ---------- ADC Setup on PC0 (Analog Pin A0) ----------
    ADMUX  = (1 << REFS0);               // Use AVcc as voltage reference, input on ADC0 (PC0)
    ADCSRA = (1 << ADEN)                 // Enable ADC
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescaler to 128 (for 16MHz clock)

    // ---------- Timer1 Setup for Servo Control on PB1 (D9) ----------
    DDRB  |= (1 << PB1);                 // Set PB1 (OC1A/D9) as output
    TCCR1A = (1 << COM1A1) | (1 << WGM11);              // Non-inverting mode, Fast PWM (part 1)
    TCCR1B = (1 << WGM13)  | (1 << WGM12) | (1 << CS11); // Fast PWM with ICR1 as TOP, prescaler = 8
    ICR1   = 39999;                      // Set TOP for 20ms period (50Hz PWM)
    OCR1A  = 3000;                       // Initial servo pulse width (~1.5ms -> centered position)

    // ---------- Timer0 Setup for LED Flashing on PB5 (D13) ----------
    DDRB  |= (1 << PB5);                 // Set PB5 (D13) as output
    TCCR0A = (1 << WGM01);              // Set CTC mode
    TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler = 1024
    OCR0A  = 15;                         // Compare match every ~1ms (16MHz / 1024 / 16 ≈ 1kHz)
    TIMSK0 = (1 << OCIE0A);             // Enable Timer0 Compare Match A interrupt

    // ---------- Timer2 Setup for PWM Brightness Control on PD3 (D3) ----------
    DDRD  |= (1 << PD3);                 // Set PD3 (OC2B/D3) as output
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20); // Fast PWM, non-inverting on OC2B
    TCCR2B = (1 << CS22)   | (1 << CS21)  | (1 << CS20);  // Prescaler = 1024
    TIMSK2 = 0;                          // No Timer2 interrupts used

    sei(); // Enable global interrupts

    // ---------- Variables for Fading Effect ----------
    uint8_t brightness   = 0;   // PWM brightness value (0-255)
    int8_t  fade_dir     = 1;   // Direction of fade (1 = brighter, -1 = dimmer)

    // ---------- ADC Stability Tracking ----------
    uint16_t last_adc       = 0;  // Last ADC reading
    uint8_t  stable_count   = 0;  // Number of consecutive stable readings
    const uint8_t stable_threshold = 10; // Stability count before fading starts

    // ---------- Fading Delay Control ----------
    uint8_t fading_started    = 0;
    uint8_t fade_delay_counter = 0;

    while (1) {
        // ---------- Read ADC from A0 ----------
        ADCSRA |= (1 << ADSC);                 // Start ADC conversion
        while (ADCSRA & (1 << ADSC));          // Wait for conversion to complete
        uint16_t adc = ADC;                    // Get result (0-1023)

        // ---------- Control Servo Position (Always Updates) ----------
        uint16_t pulse = 1000 + ((uint32_t)adc * 4000UL) / 1023UL;
        OCR1A = pulse; // Set servo pulse width between 1ms-5ms based on ADC

        // ---------- Enable Flashing if Voltage > 2.5V ----------
        flash_enable = (adc > 512); // ADC > 512 -> ~2.5V

        // ---------- Brightness Response and Fading ----------
        if (abs(adc - last_adc) > 4) {  // If ADC changed significantly
            last_adc         = adc;
            stable_count     = 0;        // Reset fading
            fading_started   = 0;
            fade_delay_counter = 0;      // Reset delay counter
            OCR2B = adc >> 2;           // Immediate PWM update (scale 0-1023 to 0-255)
        } else {
            stable_count++;

            if (stable_count > stable_threshold) {
                if (!fading_started) {
                    fade_delay_counter++;
                    if (fade_delay_counter >= 100) { // ~1 sec delay (10ms * 100)
                        fading_started = 1;
                    }
                } else {
                    // Start breathing/fading effect
                    brightness += fade_dir;
                    if (brightness == 255 || brightness == 0) {
                        fade_dir = -fade_dir; // Reverse direction at limits
                    }
                    OCR2B = brightness;
                    _delay_ms(10); // Slow down the fading
                }
            }
        }
    }
}

// ---------- Interrupt: Timer0 Compare A Match ----------
// Used to flash LED on D13 if flash_enable is set
ISR(TIMER0_COMPA_vect) {
    static uint16_t cnt    = 0;
    static uint8_t  led_on = 0;

    if (flash_enable) {
        if (++cnt >= 512) {  // Toggle every 512ms (approx.)
            cnt = 0;
            if (led_on) {
                PORTB &= ~(1 << PB5); // Turn off LED
                led_on = 0;
            } else {
                PORTB |=  (1 << PB5); // Turn on LED
                led_on = 1;
            }
        }
    } else {
        cnt    = 0;
        PORTB &= ~(1 << PB5); // Keep LED off
        led_on = 0;
    }
}
