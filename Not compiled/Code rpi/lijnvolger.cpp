//g++ -Wall -o lijn lijnvolger.cpp ~/hva_libraries/legorpi/*.cpp -I/home/piuser/hva_libraries/legorpi ~/hva_libraries/legorpi/*.c

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "rs232.h"

#define BUFSZ 4096
#define NODATA 0
#define VALIDDATA 1
#define INVALIDDATA -1

#define COMPORT 24
#define AANTAL_WAARDES 7

#define MOTOR_LEFT      PORT5_MB
#define MOTOR_RIGHT     PORT6_MA
#define MAX_SPEED       500

#define SETPOINT    0       // The goal for readLine (center)
#define KP          0.028   // The P value in PID
#define KD          3       // The D value in PID

#define MIN_LINE_CHANGE 200
#define LINE_SAMPLES 20

void exit_signal_handler(int signo);
int GetNewXMegaData(int *data_Location, int data_Size);
void rij(int* data_Location);
int check_Line_Status(int* data_Location);

int main(int nArgc, char* aArgv[]) 
{
  signal(SIGINT, exit_signal_handler);

  oLego.isDetected();
  
  int waarde[AANTAL_WAARDES];
  int commIsOpen = 0;

  commIsOpen = !RS232_OpenComport(COMPORT, 115200, "8N1", 0);
  sleep(1);
  int bende;
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  bende = GetNewXMegaData(waarde, AANTAL_WAARDES);
  
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
          int road = get_Road_Information(waarde, LINE_SAMPLES, MIN_LINE_CHANGE);
          switch(road)
          {
            case LINE:
            follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            break;

            case CROSS:
            printf("Kruising");
            follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            break;

            case SPLIT:
            printf("Splitsing");
            stop(MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            oLego.reset_all();
            exit(-2)
            break;

            case LEFT_TURN:
            printf("Bocht links");
            stop(MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            oLego.reset_all();
            exit(-2)
            break;

            case RIGHT_TURN:
            printf("Bocht rechts");
            stop(MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            oLego.reset_all();
            exit(-2)
            break;

            case NO_LINE:
            printf("Geen lijn");
            stop(MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            oLego.reset_all();
            exit(-2)
            break;

            case OPTION_LEFT:
            printf("Splitsing links");
            follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            break;

            case OPTION_RIGHT:
            printf("Splitsing rechts");
            follow_Line(waarde, SETPOINT, KP, KD, MOTOR_LEFT, MOTOR_RIGHT, MAX_SPEED);
            break;
          }
      } 
      else if(regelResult == INVALIDDATA) printf("Error\n Data niet goed ontvangen!\n");
      
    }
  }
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo) {

  if (signo == SIGINT) 
  {
    oLego.reset_all();
    printf("\nThe LEGO RPi example has stopped\n\n");
    exit(-2);
  }
}

int GetNewXMegaData(int *data_Location, int data_Size) {
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