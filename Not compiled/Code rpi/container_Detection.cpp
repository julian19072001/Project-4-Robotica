#include "container_Detection.hpp"

#define IS_THERE        0
#define IS_NOT_THERE    1

Container_t containers[MAX_NUMBER_OF_CONTAINERS];

static uint8_t number_Scanned_Containers = 0;

void save_Container(int16_t x_Pos, int16_t y_Pos, char *color);
int check_If_There(int16_t x_Pos, int16_t y_Pos);

void check_Container_Left(uint8_t left_Distance, char *left_Color, int8_t x_Direction_Modifier, int16_t x_Pos, int8_t y_Direction_Modifier, int16_t y_Pos)
{
    if(!left_Distance || left_Distance > 30) return;
    if(number_Scanned_Containers >= MAX_NUMBER_OF_CONTAINERS) return;
    static int left_Detected = 0;
    left_Detected++;
    if(left_Detected < MIN_DETECTED) return;
    left_Detected = 0;
    if(y_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, left_Color);
    }
    else if(y_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, left_Color);
    }
    else if(x_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, left_Color);
    }
    else if(x_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, left_Color);
    }
}

void check_Container_Right(uint8_t right_Distance, char *right_Color, int8_t x_Direction_Modifier, int16_t x_Pos, int8_t y_Direction_Modifier, int16_t y_Pos)
{
    if(!right_Distance || right_Distance > 30) return;
    if(number_Scanned_Containers >= MAX_NUMBER_OF_CONTAINERS) return;
    static int right_Detected = 0;
    right_Detected++;
    if(right_Detected < MIN_DETECTED) return;
    right_Detected = 0;
    if(y_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, right_Color);
    }
    else if(y_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos + 1;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, right_Color);
    }
    else if(x_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, right_Color);
    }
    else if(x_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, right_Color);
    }
}

void print_Found_Containers()
{
    printf("\nGevonden containers:\n");
    for(int i = 0; i < number_Scanned_Containers; i++)
    {
        printf("Container: %d\n", i + 1);
        printf("Coördinaat: (%d,%d)\n", containers[i].x_Pos, containers[i].y_Pos);
        printf("Kleur: %s\n\n", containers[i].color);
    } 
}

void save_Container(int16_t x_Pos, int16_t y_Pos, char *color)
{
    if(check_If_There(x_Pos, y_Pos) == IS_THERE && number_Scanned_Containers) return;
    else
    {
        containers[number_Scanned_Containers].x_Pos = x_Pos;
        containers[number_Scanned_Containers].y_Pos = y_Pos;
        strcpy(containers[number_Scanned_Containers].color, color);
        number_Scanned_Containers++;
        return;
    } 
}

int check_If_There(int16_t x_Pos, int16_t y_Pos)
{
    for(int i = 0; i < number_Scanned_Containers; i++)
    {
        if(x_Pos == containers[i].x_Pos && y_Pos == containers[i].y_Pos) return IS_THERE;
    }
    return IS_NOT_THERE;
}