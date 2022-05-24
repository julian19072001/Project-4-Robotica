#ifndef CONTAINER_HPP
#define CONTAINER_HPP

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>
    #include <sys/time.h>
    #include "Communication.h"

    #define MAX_NUMBER_OF_CONTAINERS    25
    #define MIN_DETECTED 10
    #define WAIT_FOR_COLOR 500000

    #define MAX_DISTANCE 30

    typedef struct Container
    {
        int16_t x_Pos;
        int16_t y_Pos;
        int16_t color;
    } 
    Container_t;

    void check_Container_Left(uint8_t left_Distance, int comPort, int8_t x_Direction_Modifier, int16_t x_Pos, int8_t y_Direction_Modifier, int16_t y_Pos);
    void check_Container_Right(uint8_t right_Distance, int comPort, int8_t x_Direction_Modifier, int16_t x_Pos, int8_t y_Direction_Modifier, int16_t y_Pos);
    void print_Found_Containers(int16_t x_Min, int16_t y_Min, int16_t x_Max, int16_t y_Max);

#endif