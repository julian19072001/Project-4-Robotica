#include <legorpi.hpp>
#include <unistd.h>
#include <stdbool.h>

#define LINE            0
#define NO_LINE         1
#define LEFT            2
#define RIGHT           3
#define BOTH            4

#define TURN_180        7

#define CROSS           2
#define RIGHT_TURN      3
#define LEFT_TURN       4
#define OPTION_LEFT     5
#define OPTION_RIGHT    6
#define SPLIT           7

#define STRAIGHT        1
#define TURNING_LEFT    2
#define TURNING_RIGHT   3
#define TURNING_180     4
#define STOP_DRIVING    5
#define STOP            6

#define LINE_SAMPLES    30

#define WAIT_SAMPLES    100

void follow_Line(int* data_Location, int goal, float error_Gain, uint8_t rate_Of_Change, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed);
int turn_Right(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change);
int turn_Left(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change);
int turn_180(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change);

void drive_Straight(uint8_t motor_Left, uint8_t motor_Right, uint16_t speed);
int stop(int* data_Location, uint16_t min_Line_Change, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t turning_Direction, uint16_t time);

void reset_Lego();

int check_Line_Status(int* data_Location, uint16_t min_Line_Change);
int get_Road_Information(int* data_Location, uint16_t min_Line_Change);