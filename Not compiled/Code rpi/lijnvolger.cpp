//cd ~/hva_examples/legorpi_examples
//g++ -Wall -o lijn lijnvolger.cpp ~/hva_libraries/legorpi/*.cpp -I/home/piuser/hva_libraries/legorpi ~/hva_libraries/legorpi/*.c

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <rs232.h>
#include <LineFollower.hpp>
#include <container_Detection.hpp>

#define BUFSZ 4096
#define NODATA 0
#define VALIDDATA 1
#define INVALIDDATA -1

#define SEARCH_LINE 0
#define DRIVE_OVER_GRID 1
#define GO_HOME 2
#define GO_X0 3
#define GO_Y0 4
#define TURN_0 5

#define COMPORT_DISTANCE 24
#define COMPORT_LINE 25
#define NUMBER_VALUES_DISTANCE 2
#define NUMBER_VALUES_LINE 7

#define MOTOR_LEFT      PORT6_MA
#define MOTOR_RIGHT     PORT5_MB
#define MAX_SPEED       -500
#define TURNING_SPEED   -105

#define SETPOINT    0       // The goal for readLine (center)
#define KP          0.018  // The P value in PID
#define KD          2       // The D value in PID

#define MIN_LINE_CHANGE 360

void exit_signal_handler(int signo);
int GetNewXMegaData(int comport, int *data_Location, int data_Size);

int main(int nArgc, char* aArgv[]) 
{
  signal(SIGINT, exit_signal_handler);
  
  int line_Data[NUMBER_VALUES_LINE];
  int distance_Data[NUMBER_VALUES_DISTANCE];
  
  int commIsOpen1 = 0;
  int commIsOpen2 = 0;

  commIsOpen1 = !RS232_OpenComport(COMPORT_LINE, 115200, "8N1", 0);
  commIsOpen2 = !RS232_OpenComport(COMPORT_DISTANCE, 115200, "8N1", 0);
  sleep(1);

  int program_State = SEARCH_LINE;
  int driving_State;
  int last_Junction;

  int y_Direction_Modifier;
  int y_Pos;
  int y_Max;
  int y_Min;

  int x_Direction_Modifier;
  int x_Pos;
  int x_Max;
  int x_Min;

  static bool side_Scanned;

  char tempKleur[] = "unread";
  
  while(1) 
  { 
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
    if(commIsOpen1 && commIsOpen2)
    {
      int line_Result = GetNewXMegaData(COMPORT_LINE, line_Data, NUMBER_VALUES_LINE);
      if(line_Result == VALIDDATA) 
      {
        static int just_Turned = 0;
        if(just_Turned > 0) just_Turned++;
        if(just_Turned > WAIT_SAMPLES*4) just_Turned = 0;

        int road;
        int what_Doing;
        int distance_Result;

        switch(program_State)
        {
          case SEARCH_LINE:
          road = get_Road_Information(line_Data, MIN_LINE_CHANGE);
          switch(road)
          {
            case LINE:
            follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
            break;

            case CROSS:
            printf("Kruising\n");
            x_Pos = 0;
            y_Pos = 1;
            x_Direction_Modifier = 0;
            y_Direction_Modifier = 1;
            side_Scanned = false;
            program_State = DRIVE_OVER_GRID;
            driving_State = STRAIGHT;
            break;

            case OPTION_LEFT:
            printf("Splitsing links\n");
            x_Pos = 0;
            y_Pos = 1;
            x_Direction_Modifier = 0;
            y_Direction_Modifier = 1;
            x_Max = 0;
            side_Scanned = false;
            program_State = DRIVE_OVER_GRID;
            driving_State = STRAIGHT;
            break;

            case OPTION_RIGHT:
            printf("Splitsing rechts\n");
            x_Pos = 0;
            y_Pos = 1;
            x_Direction_Modifier = 0;
            y_Direction_Modifier = 1;
            x_Min = 0;
            side_Scanned = false;
            program_State = DRIVE_OVER_GRID;
            driving_State = STRAIGHT;
            break;

            case SPLIT:
            printf("Splitsing\n");
            x_Pos = 0;
            y_Pos = 1;
            x_Direction_Modifier = 1;
            y_Direction_Modifier = 0;
            y_Max = 1;
            side_Scanned = false;
            program_State = DRIVE_OVER_GRID;
            driving_State = TURNING_RIGHT;
            break;

            case LEFT_TURN:
            printf("Bocht links\n");
            x_Pos = 0;
            y_Pos = 1;
            x_Direction_Modifier = -1;
            y_Direction_Modifier = 0;
            x_Max = 0;
            side_Scanned = true;
            program_State = DRIVE_OVER_GRID;
            driving_State = TURNING_LEFT;
            break;

            case RIGHT_TURN:
            printf("Bocht rechts\n");
            x_Pos = 0;
            y_Pos = 1;
            x_Direction_Modifier = 1;
            y_Direction_Modifier = 0;
            x_Min = 0;
            side_Scanned = false;
            program_State = DRIVE_OVER_GRID;
            driving_State = TURNING_RIGHT;
            break;

            case NO_LINE:
            printf("Error met detecteren van de lijn.\n");
            break;
          }
          break;

          case DRIVE_OVER_GRID:
          distance_Result = GetNewXMegaData(COMPORT_DISTANCE, distance_Data, NUMBER_VALUES_DISTANCE);
          if(distance_Result == VALIDDATA && !just_Turned)
          {
            if((!x_Direction_Modifier && (x_Pos != x_Min || x_Pos != x_Max)) || (!y_Direction_Modifier && (y_Pos != y_Min || y_Pos != y_Max)))
            {
              check_Container_Left(distance_Data[0], tempKleur, x_Direction_Modifier, x_Pos, y_Direction_Modifier, y_Pos);
              check_Container_Right(distance_Data[1], tempKleur, x_Direction_Modifier, x_Pos, y_Direction_Modifier, y_Pos);
            }
            else if((x_Direction_Modifier == 1 && y_Pos == y_Max) || (x_Direction_Modifier == -1 && y_Pos == y_Min) || (y_Direction_Modifier == 1 && x_Pos == x_Min) || (y_Direction_Modifier == -1 && x_Pos == x_Max))
            {
              check_Container_Right(distance_Data[1], tempKleur, x_Direction_Modifier, x_Pos, y_Direction_Modifier, y_Pos);
            }
            else if((x_Direction_Modifier == 1 && y_Pos == y_Min) || (x_Direction_Modifier == -1 && y_Pos == y_Max) || (y_Direction_Modifier == 1 && x_Pos == x_Max) || (y_Direction_Modifier == -1 && x_Pos == x_Min))
            {
              check_Container_Left(distance_Data[0], tempKleur, x_Direction_Modifier, x_Pos, y_Direction_Modifier, y_Pos);
            }
          }
          switch(driving_State)
          {
            case STRAIGHT:
            road = get_Road_Information(line_Data, MIN_LINE_CHANGE);
            switch(road)
            {
              case LINE:
              if(!just_Turned) follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
              else follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
              break;

              case CROSS:
              printf("Kruising\n");
              x_Pos += (1 * x_Direction_Modifier);
              y_Pos += (1 * y_Direction_Modifier);
              follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
              break;

              case OPTION_LEFT:
              if(!just_Turned)
              {
                printf("Splitsing links\n");
                x_Pos += (1 * x_Direction_Modifier);
                y_Pos += (1 * y_Direction_Modifier);
                if(!y_Direction_Modifier && x_Pos != 0 && (x_Pos % 2 == 0 && (side_Scanned == false || (side_Scanned == true && x_Pos < 0))))
                {
                  driving_State = TURNING_LEFT;
                  if(x_Direction_Modifier == 1)
                  {
                    x_Direction_Modifier = 0;
                    y_Direction_Modifier = 1;
                  }
                  else
                  {
                    x_Direction_Modifier = 0;
                    y_Direction_Modifier = -1;
                  }
                }
                else follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
              }
              else follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
              break;

              case OPTION_RIGHT:
              if(!just_Turned)
              {
                printf("Splitsing rechts\n");
                x_Pos += (1 * x_Direction_Modifier);
                y_Pos += (1 * y_Direction_Modifier);
                if(!y_Direction_Modifier && x_Pos != 0 && (x_Pos % 2 == 0 && (side_Scanned == false || (side_Scanned == true && x_Pos < 0))))
                {
                  driving_State = TURNING_RIGHT;
                  if(x_Direction_Modifier == 1)
                  {
                    x_Direction_Modifier = 0;
                    y_Direction_Modifier = -1;
                  }
                  else
                  {
                    x_Direction_Modifier = 0;
                    y_Direction_Modifier = 1;
                  }
                }
                else follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
              }
              else follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
              break;

              case SPLIT:
              printf("Splitsing\n");
              x_Pos += (1 * x_Direction_Modifier);
              y_Pos += (1 * y_Direction_Modifier);
              if(y_Direction_Modifier == 1)
              {
                y_Max = y_Pos;
                if(side_Scanned == true)
                {
                  x_Direction_Modifier = -1;
                  y_Direction_Modifier = 0;
                  driving_State = TURNING_LEFT;
                }
                else 
                {
                  x_Direction_Modifier = 1;
                  y_Direction_Modifier = 0;
                  driving_State = TURNING_RIGHT;
                }
              }
              else
              {
                y_Min = y_Pos;
                if(side_Scanned == true)
                {
                  x_Direction_Modifier = -1;
                  y_Direction_Modifier = 0;
                  driving_State = TURNING_RIGHT;
                }
                else 
                {
                  x_Direction_Modifier = 1;
                  y_Direction_Modifier = 0;
                  driving_State = TURNING_LEFT;
                }
              }
              break;

              case LEFT_TURN:
              printf("Bocht links\n");
              last_Junction = LEFT_TURN;
              x_Pos += (1 * x_Direction_Modifier);
              y_Pos += (1 * y_Direction_Modifier);
              if(x_Direction_Modifier == 0)
              {
                if(y_Direction_Modifier == 1)
                {
                  x_Direction_Modifier = -1;
                  y_Direction_Modifier = 0;
                }
                else
                {
                  x_Direction_Modifier = 1;
                  y_Direction_Modifier = 0;
                }
                if(side_Scanned ==  true)
                {
                  reset_Lego();
                  program_State = GO_HOME;
                  printf("Going home\n");
                } 
                else side_Scanned = true;
                driving_State = TURNING_LEFT;
              }
              else
              {
                if(x_Pos % 2 == 0)
                {
                  if(x_Direction_Modifier == 1)
                  {
                    x_Direction_Modifier = 0;
                    y_Direction_Modifier = 1;
                  }
                  else
                  {
                    x_Direction_Modifier = 0;
                    y_Direction_Modifier = -1;
                  }
                  driving_State = TURNING_LEFT;
                }
                else
                {
                  if(side_Scanned == true) 
                  {
                    reset_Lego();
                    program_State = GO_HOME;
                    x_Min = x_Pos;
                    printf("Going home\n");
                  }
                  else
                  {
                    x_Direction_Modifier = -1;
                    driving_State = TURNING_180;
                    side_Scanned = true;
                    x_Max = x_Pos;
                  }
                }
              }
              break;

              case RIGHT_TURN:
              printf("Bocht rechts\n");
              last_Junction = RIGHT_TURN;
              x_Pos += (1 * x_Direction_Modifier);
              y_Pos += (1 * y_Direction_Modifier);
              if(!x_Direction_Modifier)
              {
                if(y_Direction_Modifier == 1)
                {
                  x_Direction_Modifier = 1;
                  y_Direction_Modifier = 0;
                }
                else
                {
                  x_Direction_Modifier = -1;
                  y_Direction_Modifier = 0;
                }
                if(side_Scanned == true)
                {
                  program_State = GO_HOME;
                  printf("Going home\n");
                } 
                else if(x_Pos)side_Scanned = true;
                driving_State = TURNING_RIGHT;
              }
              else
              {
                if(x_Pos % 2 == 0)
                {
                  if(x_Direction_Modifier == 1)
                  {
                    x_Direction_Modifier = 0;
                    y_Direction_Modifier = -1;
                  }
                  else
                  {
                    x_Direction_Modifier = 0;
                    y_Direction_Modifier = 1;
                  }
                  driving_State = TURNING_RIGHT;
                }
                else
                {
                  if(side_Scanned == true) 
                  {
                    reset_Lego();
                    program_State = GO_HOME;
                    x_Min = x_Pos;
                    printf("Going home\n");
                  }
                  else
                  {
                    side_Scanned = true;
                    x_Max = x_Pos;
                    x_Direction_Modifier = -1;
                    driving_State = TURNING_180_RIGHT;
                  }
                }
              }
              break;

              case NO_LINE:
              if(just_Turned > 0)
              {
                drive_Straight(MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
              }
              else
              {
                printf("Geen lijn\n");
                reset_Lego();
                exit(-2);
              }
              break;
            }
            break;

            case TURNING_LEFT:
            what_Doing = turn_Left(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_180:
            what_Doing = turn_180(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_180) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_180_RIGHT:
            what_Doing = turn_180_Right(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_180_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;
          }
          break;

          case GO_HOME:
          switch(driving_State)
          {
            case STRAIGHT:
            if(!x_Pos && !y_Pos) program_State = TURN_0;
            else if(x_Direction_Modifier == 1)
            {
              if(x_Pos < 0) program_State = GO_X0;
              else
              {
                x_Direction_Modifier = -1;
                if(last_Junction == RIGHT_TURN) driving_State = TURNING_180_RIGHT;
                else driving_State = TURNING_180;
              } 
            }
            else if(x_Direction_Modifier == -1)
            {
              if(x_Pos > 0) program_State = GO_X0;
              else
              {
                x_Direction_Modifier = 1;
                if(last_Junction == RIGHT_TURN) driving_State = TURNING_180_RIGHT;
                else driving_State = TURNING_180;
              } 
            }
            else if(y_Direction_Modifier == 1)
            {
              if(y_Pos < 0) program_State = GO_Y0;
              else
              {
                y_Direction_Modifier = -1;
                if(last_Junction == RIGHT_TURN) driving_State = TURNING_180_RIGHT;
                else driving_State = TURNING_180;
              } 
            }
            else if(y_Direction_Modifier == -1)
            {
              if(y_Pos > 0) program_State = GO_Y0;
              else
              {
                y_Direction_Modifier = 1;
                if(last_Junction == RIGHT_TURN) driving_State = TURNING_180_RIGHT;
                else driving_State = TURNING_180;
              }
            }
            break;

            case TURNING_LEFT:
            what_Doing = turn_Left(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_180:
            what_Doing = turn_180(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_180) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_180_RIGHT:
            what_Doing = turn_180_Right(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_180_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;
          }
          break;
          
          case GO_X0:
          switch(driving_State)
          {
            case STRAIGHT:
            if(!x_Pos)
            {
              if(!y_Pos) program_State = TURN_0;
              else if(y_Pos > 0 && x_Direction_Modifier == 1)
              {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = -1;
                driving_State = TURNING_RIGHT;
              }
              else if(y_Pos > 0 && x_Direction_Modifier == -1)
              {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = -1;
                driving_State = TURNING_LEFT;
              }
              else if(y_Pos < 0 && x_Direction_Modifier == 1)
              {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = 1;
                driving_State = TURNING_LEFT;
              }
              else if(y_Pos < 0 && x_Direction_Modifier == -1)
              {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = 1;
                driving_State = TURNING_RIGHT;
              }
              else program_State = GO_Y0;
            }
            else
            {
              road = get_Road_Information(line_Data, MIN_LINE_CHANGE);
              switch(road)
              {
                case LINE:
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case CROSS:
                printf("Kruising\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case OPTION_LEFT:
                printf("Splitsing links\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case OPTION_RIGHT:
                printf("Splitsing rechts\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case SPLIT:
                printf("Splitsing\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case LEFT_TURN:
                printf("Bocht links\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case RIGHT_TURN:
                printf("Bocht rechts\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;
              }
            }
            break;

            case TURNING_LEFT:
            what_Doing = turn_Left(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;
          }
          break;

          case GO_Y0:
          switch(driving_State)
          {
            case STRAIGHT:
            if(!y_Pos)
            {
              if(!x_Pos) program_State = TURN_0;
              else if(x_Pos > 0 && y_Direction_Modifier == 1)
              {
                x_Direction_Modifier = -1;
                y_Direction_Modifier = 0;
                driving_State = TURNING_LEFT;
              }
              else if(x_Pos > 0 && y_Direction_Modifier == -1)
              {
                x_Direction_Modifier = -1;
                y_Direction_Modifier = 0;
                driving_State = TURNING_RIGHT;
              }
              else if(x_Pos < 0 && y_Direction_Modifier == 1)
              {
                x_Direction_Modifier = 1;
                y_Direction_Modifier = 0;
                driving_State = TURNING_RIGHT;
              }
              else if(x_Pos < 0 && y_Direction_Modifier == -1)
              {
                x_Direction_Modifier = 1;
                y_Direction_Modifier = 0;
                driving_State = TURNING_LEFT;
              }
              else program_State = GO_X0;
            }
            else
            {
              road = get_Road_Information(line_Data, MIN_LINE_CHANGE);
              switch(road)
              {
                case LINE:
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case CROSS:
                printf("Kruising\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case OPTION_LEFT:
                printf("Splitsing links\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case OPTION_RIGHT:
                printf("Splitsing rechts\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case SPLIT:
                printf("Splitsing\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case LEFT_TURN:
                printf("Bocht links\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;

                case RIGHT_TURN:
                printf("Bocht rechts\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(line_Data, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
                break;
              }
            }
            break;

            case TURNING_LEFT:
            what_Doing = turn_Left(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;
          }
          break;

          case TURN_0:
          switch(driving_State)
          {
            case STRAIGHT:
            if(y_Direction_Modifier == 1)
            {
              printf("Einde van programma\n");
              print_Found_Containers();
              reset_Lego();
              exit(-2);
            }
            if(y_Direction_Modifier == -1)
            {
              driving_State = TURNING_LEFT;
              y_Direction_Modifier = 0;
              x_Direction_Modifier = 1;
            }
            if(x_Direction_Modifier == 1)
            {
              driving_State = TURNING_LEFT;
              y_Direction_Modifier = 1;
              x_Direction_Modifier = 0;
            }
            if(x_Direction_Modifier == -1)
            {
              driving_State = TURNING_RIGHT;
              y_Direction_Modifier = 1;
              x_Direction_Modifier = 0;
            }
            break;

            case TURNING_LEFT:
            what_Doing = turn_Left(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(line_Data, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;
          }
          break;
        }
      } 
      //else if(regelResult == INVALIDDATA) printf("Error\n Data niet goed ontvangen!\n");    
    }
  }
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo) 
{
  if (signo == SIGINT) 
  {
    print_Found_Containers();
    reset_Lego();
    printf("\nThe line follower has stopped.\n\n");
    exit(-2);
  }
}

int GetNewXMegaData(int comport, int *data_Location, int data_Size) 
{
  static char sCommBuf[BUFSZ];
  static int sCommBufLen = 0;
  int bytesRead = 0, valid = 1, lineRead = 0;
  
  do 
  {
    bytesRead = RS232_PollComport(comport, (unsigned char *) &sCommBuf[sCommBufLen], 1);
    if(bytesRead > 0) 
    {
      sCommBufLen += bytesRead;
      sCommBuf[sCommBufLen] = '\0';

      if(sCommBuf[sCommBufLen - 1] == '\n') 
      {
        char *here = sCommBuf;
        
        for(int i = 0; i < data_Size && valid == 1; i++) 
        {
          while(isspace(*here)) here++;
          if(isdigit(*here)) data_Location[i] = strtol(here, &here, 10);
          else valid = 0;
        } 

        if(valid == 1) 
        {
          while(isspace(*here)) here++;
          if(*here != '\0') valid = 0;
          else lineRead = 1;
        }

        sCommBufLen = 0; 
      } 
      else if(sCommBufLen >= BUFSZ - 1) 
      {
        sCommBufLen = 0;
        valid = 0;
      }
    }
  } 
  while(bytesRead > 0 && lineRead == 0 && valid == 1);

  if(bytesRead == 0)      return NODATA;
  else if(lineRead == 1)  return VALIDDATA;
  else                    return INVALIDDATA;
}