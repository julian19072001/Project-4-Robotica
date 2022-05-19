#ifndef CONTAINER_HPP
#define CONTAINER_HPP

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>

    #define MAX_NUMBER_OF_CONTAINERS    25
    #define MIN_DETECTED 15

    typedef struct Container
    {
        int16_t x_Pos;
        int16_t y_Pos;
        char color[16];
    } 
    Container_t;

    void check_Container_Left(uint8_t left_Distance, char *left_Color, int8_t x_Direction_Modifier, int16_t x_Pos, int8_t y_Direction_Modifier, int16_t y_Pos);
    void check_Container_Right(uint8_t right_Distance, char *right_Color, int8_t x_Direction_Modifier, int16_t x_Pos, int8_t y_Direction_Modifier, int16_t y_Pos);
    void print_Found_Containers(int16_t x_Min, int16_t y_Min, int16_t x_Max, int16_t y_Max);

#endif