#ifndef F_CPU 
#define F_CPU 32000000UL
#endif

#ifndef ULTRASONIC_HPP
#define ULTRASONIC_HPP

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define DEFAULT_MAX_DISTANCE    400
#define DEFAULT_MAX_TIMEOUT     0
#define DEFAULT_TRIGGER_PIN     PIN0_bm
#define DEFAULT_ECHO_PIN        PIN1_bm
#define DEFAULT_PORT            &PORTD

class Ultrasonic
{
    public:
    Ultrasonic();
    Ultrasonic(uint16_t max_Distance, uint64_t max_Timeout);
    Ultrasonic(PORT_t *port, uint8_t trigger_Pin, uint8_t echo_Pin);
    Ultrasonic(uint16_t max_Distance, uint64_t max_Timeout, PORT_t *port, uint8_t trigger_Pin, uint8_t echo_Pin);

    float Measure_distance_cm(float temperature);

    private:
    uint16_t max_distance_cm;
    uint64_t max_timeout_us;

    PORT_t *port_c;
    uint8_t trigger_Pin_c;
    uint8_t echo_Pin_c;
};

#endif 