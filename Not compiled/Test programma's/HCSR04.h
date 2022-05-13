#ifndef HCSR04_H
#define HCSR04_H

    void Init_HCSR04(uint16_t max_dis, uint64_t max_timeout);
    float Measure_distance_cm(float temperature);
    
#endif // HCSR04_H