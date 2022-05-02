#include <unistd.h>
#include <stdbool.h>
#include "legorpi.hpp"

#define LINE            0
#define NO_LINE         1
#define LEFT            2
#define RIGHT           3
#define BOTH            4

#define CROSS           2
#define RIGHT_TURN      3
#define LEFT_TURN       4
#define OPTION_LEFT     5
#define OPTION_RIGHT    6
#define SPLIT           7

void follow_Line(int* data_Location, int goal, float error_Gain, uint8_t rate_Of_Change, uint8_t motor_Left, uint8_t motor_Right, uint8_t speed);
void turn_Right(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint8_t speed);
void turn_Left(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint8_t speed);
void turn_180(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint8_t speed);
void stop(uint8_t motor_Left, uint8_t motor_Right, uint8_t speed);

int check_Line_Status(int* data_Location, uint8_t line_Samples, uint8_t min_Line_Change);
int get_Road_Information(int* data_Location, uint8_t line_Samples, uint8_t min_Line_Change);