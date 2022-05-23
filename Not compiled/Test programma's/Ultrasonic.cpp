#include "Ultrasonic.hpp"

void Init_HCSR04_timer(void);
void Stop_HCSR04_timer(void);

volatile uint64_t time_us;

Ultrasonic::Ultrasonic() {
    max_distance_cm     = DEFAULT_MAX_DISTANCE;
    max_timeout_us      = DEFAULT_MAX_TIMEOUT;
    port_c              = DEFAULT_PORT;
    echo_Pin_c          = DEFAULT_ECHO_PIN;
    trigger_Pin_c       = DEFAULT_TRIGGER_PIN;

    time_us = 0;
    port_c->DIRCLR = echo_Pin_c;		    // ECHO
	port_c->DIRSET = trigger_Pin_c;		    // TRIG

    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei(); 
}

Ultrasonic::Ultrasonic(uint16_t max_Distance, uint64_t max_Timeout) 
{
    max_distance_cm     = max_Distance;
    max_timeout_us      = max_Timeout;
    port_c              = DEFAULT_PORT;
    echo_Pin_c          = DEFAULT_ECHO_PIN;
    trigger_Pin_c       = DEFAULT_TRIGGER_PIN;
    
    time_us = 0;
    port_c->DIRCLR = echo_Pin_c;		    // ECHO
	port_c->DIRSET = trigger_Pin_c;		    // TRIG

    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei(); 
}

Ultrasonic::Ultrasonic(PORT_t *port, uint8_t trigger_Pin, uint8_t echo_Pin)
{
    max_distance_cm     = DEFAULT_MAX_DISTANCE;
    max_timeout_us      = DEFAULT_MAX_TIMEOUT;
    port_c              = port;
    echo_Pin_c          = echo_Pin;
    trigger_Pin_c       = trigger_Pin;
    
    time_us = 0;
    port_c->DIRCLR = PIN1_bm;		    // ECHO
	port_c->DIRSET = PIN0_bm;		    // TRIG

    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei(); 
}

Ultrasonic::Ultrasonic(uint16_t max_Distance, uint64_t max_Timeout, PORT_t *port, uint8_t trigger_Pin, uint8_t echo_Pin) 
{
    max_distance_cm     = max_Distance;
    max_timeout_us      = max_Timeout;
    port_c              = port;
    echo_Pin_c          = echo_Pin;
    trigger_Pin_c       = trigger_Pin;
    
    time_us = 0;
    port_c->DIRCLR = echo_Pin_c;		    // ECHO
	port_c->DIRSET = trigger_Pin_c;		    // TRIG

    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei(); 
}

void Init_Ultrasonic_timer(void) 
{
    TCE1.PER      = 31;     				    // Tper =  1 * (31 + 1) / 32M = 0.000001 s
    TCE1.CTRLA    = TC_CLKSEL_DIV1_gc;          // Prescaling 1
    TCE1.CTRLB    = TC_WGMODE_NORMAL_gc;        // Normal mode
    TCE1.INTCTRLA = TC_OVFINTLVL_LO_gc;        	// Interrupt overflow off
}

void Stop_Ultrasonic_timer(void) 
{
	TCE1.CTRLA    = TC_CLKSEL_OFF_gc;			// timer/counter off
    TCE1.INTCTRLA = TC_OVFINTLVL_OFF_gc;		// disables overflow interrupt
}

uint16_t Ultrasonic::Measure_distance_cm(float temperature) 
{
    uint64_t max_distance_duration_us;
    uint64_t duration_us;

    // Define speed of sound in cm per second
    float speed_of_sound_cm_per_us = 0.03313 + 0.0000606 * temperature; // Cair ≈ (331.3 + 0.606 ⋅ ϑ) m/s

    // Compute max delay based on max distance with 25% margin in microseconds
    max_distance_duration_us = 2.5 * (float)max_distance_cm / speed_of_sound_cm_per_us;
    if(max_timeout_us > 0) 
    {
        if(max_distance_duration_us < max_timeout_us)
        {
            max_distance_duration_us = max_timeout_us;
        }
    }

    port_c->OUTCLR = trigger_Pin_c;		// TRIG low
    _delay_us(40);

    // Hold trigger for 10 microseconds, which is signal for sensor to measure distance.
    port_c->OUTSET = trigger_Pin_c;		// TRIG high
    _delay_us(2000);
    port_c->OUTCLR = trigger_Pin_c;		// TRIG low

    // Measure the length of echo signal, which is equal to the time needed for sound to go there and back.
    time_us = 0;
    Init_Ultrasonic_timer();

    switch(echo_Pin_c)
    {
        case PIN0_bm:
        while((bit_is_clear(port_c->IN, PIN0_bp)) && (time_us < max_distance_duration_us));
        time_us = 0;

        while((bit_is_set(port_c->IN, PIN0_bp)) && (time_us < max_distance_duration_us));
        Stop_Ultrasonic_timer();
        break;

        case PIN1_bm:
        while((bit_is_clear(port_c->IN, PIN1_bp)) && (time_us < max_distance_duration_us));
        time_us = 0;

        while((bit_is_set(port_c->IN, PIN1_bp)) && (time_us < max_distance_duration_us));
        Stop_Ultrasonic_timer();
        break;

        case PIN2_bm:
        while((bit_is_clear(port_c->IN, PIN2_bp)) && (time_us < max_distance_duration_us));
        time_us = 0;

        while((bit_is_set(port_c->IN, PIN2_bp)) && (time_us < max_distance_duration_us));
        Stop_Ultrasonic_timer();
        break;

        case PIN3_bm:
        while((bit_is_clear(port_c->IN, PIN3_bp)) && (time_us < max_distance_duration_us));
        time_us = 0;

        while((bit_is_set(port_c->IN, PIN3_bp)) && (time_us < max_distance_duration_us));
        Stop_Ultrasonic_timer();
        break;

        case PIN4_bm:
        while((bit_is_clear(port_c->IN, PIN4_bp)) && (time_us < max_distance_duration_us));
        time_us = 0;

        while((bit_is_set(port_c->IN, PIN4_bp)) && (time_us < max_distance_duration_us));
        Stop_Ultrasonic_timer();
        break;

        case PIN5_bm:
        while((bit_is_clear(port_c->IN, PIN5_bp)) && (time_us < max_distance_duration_us));
        time_us = 0;

        while((bit_is_set(port_c->IN, PIN5_bp)) && (time_us < max_distance_duration_us));
        Stop_Ultrasonic_timer();
        break;

        case PIN6_bm:
        while((bit_is_clear(port_c->IN, PIN6_bp)) && (time_us < max_distance_duration_us));
        time_us = 0;

        while((bit_is_set(port_c->IN, PIN6_bp)) && (time_us < max_distance_duration_us));
        Stop_Ultrasonic_timer();
        break;

        case PIN7_bm:
        while((bit_is_clear(port_c->IN, PIN7_bp)) && (time_us < max_distance_duration_us));
        time_us = 0;

        while((bit_is_set(port_c->IN, PIN7_bp)) && (time_us < max_distance_duration_us));
        Stop_Ultrasonic_timer();
        break;
    }

    if(time_us < max_distance_duration_us) 
    {
        duration_us = time_us;
    } 
    else 
    {
        duration_us = 0;
    }

    uint16_t distance_cm = duration_us / 2.0 * speed_of_sound_cm_per_us * 3.6;

    if(!distance_cm || (distance_cm > max_distance_cm)) 
    {
        return 0;
    } 
    else 
    {
        return distance_cm;
    }
}

ISR (TCE1_OVF_vect) 
{
    time_us++;
}