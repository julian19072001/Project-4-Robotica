//cd ~/hva_examples/legorpi_examples
//g++ -Wall -o lijn lijnvolger.cpp ~/hva_libraries/legorpi/*.cpp -I/home/piuser/hva_libraries/legorpi ~/hva_libraries/legorpi/*.c

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <rs232.h>
#include <LineFollower.hpp>

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

#define COMPORT 24
#define AANTAL_WAARDES 7

#define MOTOR_LEFT      PORT6_MA
#define MOTOR_RIGHT     PORT5_MB
#define MAX_SPEED       -500
#define TURNING_SPEED   -250
#define REVERSE_TIME    100

#define SETPOINT    0       // The goal for readLine (center)
#define KP          0.028   // The P value in PID
#define KD          3       // The D value in PID

#define MIN_LINE_CHANGE 500

void exit_signal_handler(int signo);
int GetNewXMegaData(int *data_Location, int data_Size);

int main(int nArgc, char* aArgv[]) 
{
  signal(SIGINT, exit_signal_handler);
  
  int waarde[AANTAL_WAARDES];
  int commIsOpen = 0;

  commIsOpen = !RS232_OpenComport(COMPORT, 115200, "8N1", 0);
  sleep(1);

  int program_State = SEARCH_LINE;
  int driving_State = STRAIGHT;
  int target_Direction;

  int y_Direction_Modifier;
  int y_Pos;
  int y_Max;
  int y_Min;

  int x_Direction_Modifier;
  int x_Pos;
  int x_Max;
  int x_Min;

  static bool right_Scanned;
  
  while(1) 
  { 
    if (!commIsOpen) 
    {
      printf("Can not open COM %d\nExit Program\n", COMPORT);
      exit(-2);
    }
    if(commIsOpen)
    {
      int regelResult = GetNewXMegaData(waarde, AANTAL_WAARDES);
      if(regelResult == VALIDDATA) 
      {
        static int just_Turned = 0;
        if(just_Turned > 0) just_Turned++;
        if(just_Turned > WAIT_SAMPLES) just_Turned = 0;

        int road;
        int what_Doing;

        switch(program_State)
        {
          case SEARCH_LINE:
          road = get_Road_Information(waarde, MIN_LINE_CHANGE);
          switch(road)
          {
            case LINE:
            follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            break;

            case CROSS:
            printf("Kruising\n");
            x_Pos = 0;
            y_Pos = 0;
            x_Direction_Modifier = 0;
            y_Direction_Modifier = 1;
            right_Scanned = false;
            program_State = DRIVE_OVER_GRID;
            break;
          }
          break;

          case DRIVE_OVER_GRID:
          switch(driving_State)
          {
            case STRAIGHT:
            road = get_Road_Information(waarde, MIN_LINE_CHANGE);
            switch(road)
            {
              case LINE:
              follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
              break;

              case CROSS:
              printf("Kruising\n");
              x_Pos += (1 * x_Direction_Modifier);
              y_Pos += (1 * y_Direction_Modifier);
              follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
              break;

              case OPTION_LEFT:
              printf("Splitsing links\n");
              x_Pos += (1 * x_Direction_Modifier);
              y_Pos += (1 * y_Direction_Modifier);
              if(!y_Direction_Modifier && x_Pos != 0 && (x_Pos % 2 == 0 && (right_Scanned == false || (right_Scanned == true && x_Pos < 0))))
              {
                target_Direction = LEFT;
                driving_State = STOP;
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
              else follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
              break;

              case OPTION_RIGHT:
              printf("Splitsing rechts\n");
              x_Pos += (1 * x_Direction_Modifier);
              y_Pos += (1 * y_Direction_Modifier);
              if(!y_Direction_Modifier && x_Pos != 0 && (x_Pos % 2 == 0 && (right_Scanned == false || (right_Scanned == true && x_Pos < 0))))
              {
                target_Direction = RIGHT;
                driving_State = STOP;
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
              else follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
              break;

              case SPLIT:
              printf("Splitsing\n");
              x_Pos += (1 * x_Direction_Modifier);
              y_Pos += (1 * y_Direction_Modifier);
              if(y_Direction_Modifier == 1)
              {
                y_Max = y_Pos;
                if(right_Scanned == true)
                {
                  x_Direction_Modifier = -1;
                  y_Direction_Modifier = 0;
                  target_Direction = LEFT;
                  driving_State = STOP;
                }
                else 
                {
                  x_Direction_Modifier = 1;
                  y_Direction_Modifier = 0;
                  target_Direction = RIGHT;
                  driving_State = STOP;
                }
              }
              else
              {
                y_Min = y_Pos;
                if(right_Scanned == true)
                {
                  x_Direction_Modifier = -1;
                  y_Direction_Modifier = 0;
                  target_Direction = RIGHT;
                  driving_State = STOP;
                }
                else 
                {
                  x_Direction_Modifier = 1;
                  y_Direction_Modifier = 0;
                  target_Direction = LEFT;
                  driving_State = STOP;
                }
              }
              break;

              case LEFT_TURN:
              printf("Bocht links\n");
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
                if(right_Scanned ==  true)
                {
                  target_Direction = STOP_DRIVING;
                  driving_State = STOP;
                  program_State = GO_HOME;
                  printf("Going home\n");
                } 
                else right_Scanned = true;
                target_Direction = LEFT;
                driving_State = STOP;
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
                  target_Direction = LEFT;
                  driving_State = STOP;
                }
                else
                {
                  if(right_Scanned == true) 
                  {
                    target_Direction = STOP_DRIVING;
                    driving_State = STOP;
                    program_State = GO_HOME;
                    x_Min = x_Pos;
                    printf("Going home\n");
                  }
                  else
                  {
                    x_Direction_Modifier = -1;
                    target_Direction = TURN_180;
                    driving_State = STOP;
                    right_Scanned = true;
                    x_Max = x_Pos;
                  }
                }
              }
              break;

              case RIGHT_TURN:
              printf("Bocht rechts\n");
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
                if(right_Scanned ==  true)
                {
                  program_State = GO_HOME;
                  printf("Going home\n");
                } 
                else right_Scanned = true;
                target_Direction = RIGHT;
                driving_State = STOP;
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
                  target_Direction = RIGHT;
                  driving_State = STOP;
                }
                else
                {
                  if(right_Scanned == true) 
                  {
                    target_Direction = STOP_DRIVING;
                    driving_State = STOP;
                    program_State = GO_HOME;
                    x_Min = x_Pos;
                    printf("Going home\n");
                  }
                  else
                  {
                    right_Scanned = true;
                    x_Max = x_Pos;
                    x_Direction_Modifier = -1;
                    target_Direction = TURN_180;
                    driving_State = STOP;
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
            what_Doing = turn_Left(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_180:
            what_Doing = turn_180(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_180) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case STOP:
            driving_State = stop(waarde, MIN_LINE_CHANGE, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, target_Direction, REVERSE_TIME);
            break;

            case STOP_DRIVING:
            reset_Lego();
            break;
          }
          break;

          case GO_HOME:
          switch(driving_State)
          {
            case STRAIGHT:
            if(x_Direction_Modifier == 1)
            {
              if(x_Pos < 0) program_State = GO_X0;
              else
              {
                x_Direction_Modifier = -1;
                driving_State = TURNING_180;
              } 
            }
            else if(x_Direction_Modifier == -1)
            {
              if(x_Pos > 0) program_State = GO_X0;
              else
              {
                x_Direction_Modifier = 1;
                driving_State = TURNING_180;
              } 
            }
            else if(y_Direction_Modifier == 1)
            {
              if(y_Pos < 0) program_State = GO_Y0;
              else
              {
                y_Direction_Modifier = -1;
                driving_State = TURNING_180;
              } 
            }
            else if(y_Direction_Modifier == -1)
            {
              if(y_Pos > 0) program_State = GO_Y0;
              else
              {
                y_Direction_Modifier = 1;
                driving_State = TURNING_180;
              }
            }
            break;

            case TURNING_180:
            what_Doing = turn_180(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_180) driving_State = what_Doing;
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
                target_Direction = RIGHT;
                driving_State = STOP;
              }
              else if(y_Pos > 0 && x_Direction_Modifier == -1)
              {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = -1;
                target_Direction = LEFT;
                driving_State = STOP;
              }
              else if(y_Pos < 0 && x_Direction_Modifier == 1)
              {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = 1;
                target_Direction = LEFT;
                driving_State = STOP;
              }
              else if(y_Pos < 0 && x_Direction_Modifier == -1)
              {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = 1;
                target_Direction = RIGHT;
                driving_State = STOP;
              }
              else program_State = GO_Y0;
            }
            else
            {
              road = get_Road_Information(waarde, MIN_LINE_CHANGE);
              switch(road)
              {
                case LINE:
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;

                case CROSS:
                printf("Kruising\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;

                case OPTION_LEFT:
                printf("Splitsing links\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;

                case OPTION_RIGHT:
                printf("Splitsing links\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;

                case SPLIT:
                printf("Splitsing links\n");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;
              }
            }
            break;

            case TURNING_LEFT:
            what_Doing = turn_Left(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case STOP:
            driving_State = stop(waarde, MIN_LINE_CHANGE, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, target_Direction, REVERSE_TIME);
            break;

            case STOP_DRIVING:
            reset_Lego();
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
                target_Direction = LEFT;
                driving_State = STOP;
              }
              else if(x_Pos > 0 && y_Direction_Modifier == -1)
              {
                x_Direction_Modifier = -1;
                y_Direction_Modifier = 0;
                target_Direction = RIGHT;
                driving_State = STOP;
              }
              else if(x_Pos < 0 && y_Direction_Modifier == 1)
              {
                x_Direction_Modifier = 1;
                y_Direction_Modifier = 0;
                target_Direction = RIGHT;
                driving_State = STOP;
              }
              else if(x_Pos < 0 && y_Direction_Modifier == -1)
              {
                x_Direction_Modifier = 1;
                y_Direction_Modifier = 0;
                target_Direction = LEFT;
                driving_State = STOP;
              }
              else program_State = GO_X0;
            }
            else
            {
              road = get_Road_Information(waarde, MIN_LINE_CHANGE);
              switch(road)
              {
                case LINE:
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;

                case CROSS:
                printf("Kruising\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;

                case OPTION_LEFT:
                printf("Splitsing links\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;

                case OPTION_RIGHT:
                printf("Splitsing links\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;

                case SPLIT:
                printf("Splitsing links\n");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
                break;
              }
            }
            break;

            case TURNING_LEFT:
            what_Doing = turn_Left(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(waarde, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case STOP:
            driving_State = stop(waarde, MIN_LINE_CHANGE, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, target_Direction, REVERSE_TIME);
            break;

            case STOP_DRIVING:
            reset_Lego();
            break;
          }
          break;

          case TURN_0:
          switch(driving_State)
          {
            case STRAIGHT:
            if(y_Direction_Modifier == 1)
            {
              printf("Einde van programma");
              reset_Lego();
              exit(-2);
            }
            if(y_Direction_Modifier == -1)
            {
              target_Direction = TURN_180;
              driving_State = STOP;
              y_Direction_Modifier = 1;
            }
            if(x_Direction_Modifier == 1)
            {
              target_Direction = RIGHT;
              driving_State = STOP;
              y_Direction_Modifier = 1;
              x_Direction_Modifier = 0;
            }
            if(x_Direction_Modifier == -1)
            {
              target_Direction = LEFT;
              driving_State = STOP;
              y_Direction_Modifier = 1;
              x_Direction_Modifier = 0;
            }
            break;

            case TURNING_LEFT:
            what_Doing = turn_Left(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_RIGHT:
            what_Doing = turn_Right(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case TURNING_180:
            what_Doing = turn_180(waarde, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, MIN_LINE_CHANGE);
            if(what_Doing == TURNING_180) driving_State = what_Doing;
            else
            {
              just_Turned = 1;
              driving_State = what_Doing;
            }
            break;

            case STOP:
            driving_State = stop(waarde, MIN_LINE_CHANGE, MOTOR_LEFT, MOTOR_RIGHT, TURNING_SPEED, target_Direction, REVERSE_TIME);
            break;

            case STOP_DRIVING:
            reset_Lego();
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
    reset_Lego();
    printf("\nThe line follower has stopped.\n\n");
    exit(-2);
  }
}

int GetNewXMegaData(int *data_Location, int data_Size) 
{
  static char sCommBuf[BUFSZ];
  static int sCommBufLen = 0;
  int bytesRead = 0, valid = 1, lineRead = 0;
  
  do 
  {
    bytesRead = RS232_PollComport(COMPORT, (unsigned char *) &sCommBuf[sCommBufLen], 1);
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