#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <Communication.h>
#include <Line_Algorithm.hpp>
#include <UI.hpp>

#define COMPORT_DISTANCE         24
#define COMPORT_LINE             25
#define COMPORT_LEFT             24
#define COMPORT_RIGHT            24

#define NUMBER_VALUES_DISTANCE   2
#define NUMBER_VALUES_LINE       7

#define MOTOR_LEFT               PORT6_MA
#define MOTOR_RIGHT              PORT5_MB
#define MAX_SPEED                500
#define TURNING_SPEED            105

#define SETPOINT                 770   
#define KP                       0.018  
#define KD                       1      

#define MIN_SIDE_LINE_CHANGE     340
#define MIN_MID_LINE_CHANGE      1025

#define LINE_SAMPLES             20
#define WAIT_SAMPLES             115

#define MAX_CONTAINER_DISTANCE   30
#define MAX_NUMBER_OF_CONTAINERS 25

#define MAX_RETRIES              1

static int distance_Data[NUMBER_VALUES_DISTANCE];

algorithm follower;

void distance_Reading();
void exit_signal_handler(int signo);

int main(int nArgc, char* aArgv[]) 
{
  signal(SIGINT, exit_signal_handler);

  int number_Of_Expected_Containers;

  if(nArgc != 2)
  {
    number_Of_Expected_Containers = 0;
  }
  else sscanf(aArgv[1], "%d", &number_Of_Expected_Containers);

  if(number_Of_Expected_Containers > MAX_NUMBER_OF_CONTAINERS) 
  {
    printf("Je wilt meer containers scannen dan het maximale aantal: %d\n", MAX_NUMBER_OF_CONTAINERS);
    exit(-2);
  }
  else if(number_Of_Expected_Containers < 0)
  {
    printf("Je kan geen negatief aantal containers scannen!\n");
    exit(-2);
  }

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

  while(GetNewXMegaData(COMPORT_LINE, line_Data, NUMBER_VALUES_LINE) != VALIDDATA);

  for(int i = 0; i > 100; i++)
  {
    check_Line_Status(line_Data, MIN_SIDE_LINE_CHANGE, MIN_MID_LINE_CHANGE, LINE_SAMPLES);
  }

  Startup_UI();

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
          if(follower.reached_Y_Min == false || follower.reached_Y_Max == false) distance_Reading();
          program_State = follower.go_Y0();
          break;

          case TURN_0:
          program_State = follower.turn_0(number_Of_Expected_Containers, MAX_RETRIES);
          break;
        }
      }  
    }
  }
}

void distance_Reading()
{
    if(follower.y_Max && follower.driving_State == STRAIGHT)
    {
        int distance_Result = GetNewXMegaData(COMPORT_DISTANCE, distance_Data, NUMBER_VALUES_DISTANCE);
        if(distance_Result == VALIDDATA && (!follower.just_Turned || follower.just_Turned > (follower.wait_Samples_c*3.5)))
        {
            if((!follower.x_Direction_Modifier && (follower.x_Pos != follower.x_Min && follower.x_Pos != follower.x_Max)) || (!follower.y_Direction_Modifier && (follower.y_Pos != follower.y_Min && follower.y_Pos != follower.y_Max)))
            {
                check_Container_Left(distance_Data[0], MAX_CONTAINER_DISTANCE, COMPORT_LEFT, follower.x_Direction_Modifier, follower.x_Pos, follower.y_Direction_Modifier, follower.y_Pos);
                check_Container_Right(distance_Data[1], MAX_CONTAINER_DISTANCE, COMPORT_RIGHT, follower.x_Direction_Modifier, follower.x_Pos, follower.y_Direction_Modifier, follower.y_Pos);
            }
            else if((follower.x_Direction_Modifier == 1 && follower.y_Pos == follower.y_Max) || (follower.x_Direction_Modifier == -1 && follower.y_Pos == follower.y_Min) || (follower.y_Direction_Modifier == 1 && follower.x_Pos == follower.x_Min) || (follower.y_Direction_Modifier == -1 && follower.x_Pos == follower.x_Max))
            {
                check_Container_Right(distance_Data[1], MAX_CONTAINER_DISTANCE, COMPORT_RIGHT, follower.x_Direction_Modifier, follower.x_Pos, follower.y_Direction_Modifier, follower.y_Pos);
            }
            else if((follower.x_Direction_Modifier == 1 && follower.y_Pos == follower.y_Min) || (follower.x_Direction_Modifier == -1 && follower.y_Pos == follower.y_Max) || (follower.y_Direction_Modifier == 1 && follower.x_Pos == follower.x_Max) || (follower.y_Direction_Modifier == -1 && follower.x_Pos == follower.x_Min))
            {
                check_Container_Left(distance_Data[0], MAX_CONTAINER_DISTANCE, COMPORT_LEFT, follower.x_Direction_Modifier, follower.x_Pos, follower.y_Direction_Modifier, follower.y_Pos);
            }
        }
    }
}

void exit_signal_handler(int signo) 
{
  if (signo == SIGINT) 
  {
    follower.stop();
  }
}