//cd ~/hva_examples/legorpi_examples
//g++ -Wall -o lijn lijnvolger.cpp ~/hva_libraries/legorpi/*.cpp -I/home/piuser/hva_libraries/legorpi ~/hva_libraries/legorpi/*.c

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <Communication.h>
#include <Line_Algorithm.hpp>

#define COMPORT_DISTANCE        24
#define COMPORT_LINE            25
#define COMPORT_LEFT            27
#define COMPORT_RIGHT           26

#define NUMBER_VALUES_DISTANCE  2
#define NUMBER_VALUES_LINE      7

#define MOTOR_LEFT              PORT6_MA
#define MOTOR_RIGHT             PORT5_MB
#define MAX_SPEED               500
#define TURNING_SPEED           105

#define SETPOINT                770    // The goal for readLine (center)
#define KP                      0.018  // The P value in PID
#define KD                      1      // The D value in PID

#define MIN_SIDE_LINE_CHANGE    340
#define MIN_MID_LINE_CHANGE     1025

#define LINE_SAMPLES            20
#define WAIT_SAMPLES            115

static int distance_Data[NUMBER_VALUES_DISTANCE];

algorithm follower;

void distance_Reading();
void exit_signal_handler(int signo);

int main(int nArgc, char* aArgv[]) 
{
  signal(SIGINT, exit_signal_handler);
  
  int commIsOpen1 = 0;
  int commIsOpen2 = 0;

  commIsOpen1 = !RS232_OpenComport(COMPORT_LINE, 115200, "8N1", 0);
  commIsOpen2 = !RS232_OpenComport(COMPORT_DISTANCE, 115200, "8N1", 0);
  
  sleep(1);
  
  if (!commIsOpen1) 
  {
    printf("Can not open COM%d for line follower\nExit Program\n", COMPORT_LINE);
    exit(-2);
  }
  if (!commIsOpen2) 
  {
    printf("Can not open COM%d for distance reader\nExit Program\n", COMPORT_DISTANCE);
    exit(-2);
  }

  int line_Data[NUMBER_VALUES_LINE];
  
  int program_State = SEARCH_LINE;

  follower.setup_Motor(MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, TURNING_SPEED);
  follower.setup_PID(line_Data, SETPOINT, KP, KD);
  follower.setup_Samples(LINE_SAMPLES, WAIT_SAMPLES);
  follower.setup_Contrast(MIN_SIDE_LINE_CHANGE, MIN_MID_LINE_CHANGE);

  for(int i = 0; i > 100; i++)
  {
    int trash = GetNewXMegaData(COMPORT_LINE, line_Data, NUMBER_VALUES_LINE);
    usleep(10000);
  }

  for(int i = 0; i > LINE_SAMPLES * 2; i++)
  {
    check_Line_Status(line_Data, MIN_SIDE_LINE_CHANGE, MIN_MID_LINE_CHANGE, LINE_SAMPLES);
  }
  
  while(1) 
  { 
    if(commIsOpen1 && commIsOpen2)
    {    
      int line_Result = GetNewXMegaData(COMPORT_LINE, line_Data, NUMBER_VALUES_LINE);
      if(line_Result == VALIDDATA) 
      {
        switch(program_State)
        {
          case SEARCH_LINE:
          program_State = follower.search_Line();
          break;

          case DRIVE_OVER_GRID:
          distance_Reading();
          program_State = follower.drive_Over_Grid();
          break;

          case GO_HOME:
          program_State = follower.go_Home();
          break;
          
          case GO_X0:
          distance_Reading();
          program_State = follower.go_X0();
          break;

          case GO_Y0:
          distance_Reading();
          program_State = follower.go_Y0();
          break;

          case TURN_0:
          follower.turn_0();
          break;
        }
      }  
    }
  }
}

void distance_Reading()
{
    int distance_Result = GetNewXMegaData(COMPORT_DISTANCE, distance_Data, NUMBER_VALUES_DISTANCE);
    if(distance_Result == VALIDDATA && !follower.just_Turned)
    {
        if((!follower.x_Direction_Modifier && (follower.x_Pos != follower.x_Min && follower.x_Pos != follower.x_Max)) || (!follower.y_Direction_Modifier && (follower.y_Pos != follower.y_Min && follower.y_Pos != follower.y_Max)))
        {
            check_Container_Left(distance_Data[0], COMPORT_LEFT, follower.x_Direction_Modifier, follower.x_Pos, follower.y_Direction_Modifier, follower.y_Pos);
            check_Container_Right(distance_Data[1], COMPORT_RIGHT, follower.x_Direction_Modifier, follower.x_Pos, follower.y_Direction_Modifier, follower.y_Pos);
        }
        else if((follower.x_Direction_Modifier == 1 && follower.y_Pos == follower.y_Max) || (follower.x_Direction_Modifier == -1 && follower.y_Pos == follower.y_Min) || (follower.y_Direction_Modifier == 1 && follower.x_Pos == follower.x_Min) || (follower.y_Direction_Modifier == -1 && follower.x_Pos == follower.x_Max))
        {
            check_Container_Right(distance_Data[1], COMPORT_RIGHT, follower.x_Direction_Modifier, follower.x_Pos, follower.y_Direction_Modifier, follower.y_Pos);
        }
        else if((follower.x_Direction_Modifier == 1 && follower.y_Pos == follower.y_Min) || (follower.x_Direction_Modifier == -1 && follower.y_Pos == follower.y_Max) || (follower.y_Direction_Modifier == 1 && follower.x_Pos == follower.x_Max) || (follower.y_Direction_Modifier == -1 && follower.x_Pos == follower.x_Min))
        {
            check_Container_Left(distance_Data[0], COMPORT_LEFT, follower.x_Direction_Modifier, follower.x_Pos, follower.y_Direction_Modifier, follower.y_Pos);
        }
    }
}

//Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo) 
{
  if (signo == SIGINT) 
  {
    follower.stop();
  }
}