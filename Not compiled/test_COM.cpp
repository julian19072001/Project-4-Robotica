#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <legorpi.hpp>
#include <rs232.h>

#define BUFSZ 4096
#define COMPORT 24
#define NODATA 0
#define VALIDDATA 1
#define INVALIDDATA -1
#define AANTAL_WAARDES 7

// Create a BrickPi3 instance with the default address of 1
BrickPi3 oLego;

//Method to catch the CTRL+C signal.
void exit_signal_handler(int signo);
//Get new data form the xMega
int GetNewXMegaData(int *data_Location, int data_Size);

// Start of the program
int main(int nArgc, char* aArgv[]) {
  // Register the exit function for Ctrl+C
  signal(SIGINT, exit_signal_handler);
  printf("Test1");
  // Make sure that the BrickPi3 is communicating and the firmware is working as expected.
  oLego.isDetected();
  
  int waarde[AANTAL_WAARDES];
   
   while(true) {
     int regelResult = GetNewXMegaData(waarde, AANTAL_WAARDES);
     if(regelResult == VALIDDATA) {
        printf("Succes! Waardes zijn:");
        for(int i = 0; i < AANTAL_WAARDES; i++)
        printf(" %d", waarde[i]);
        printf("\n");
     } else if(regelResult == INVALIDDATA) {
        printf("Error");
     };/* en als regelResult == 0, dan was er nog geen nieuwe regel binnen */
   };
};

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo) {

  if (signo == SIGINT) {
    // Reset everything so there are no run-away motors
    oLego.reset_all();
    printf("\nThe LEGO RPi example has stopped\n\n");
    exit(-2);
  };
};

//Get new data form the xMega
int GetNewXMegaData(int *data_Location, int data_Size) 
{
  static char sCommBuf[BUFSZ];
  static char sCommBufLen = 0;
  int bytesRead = 0, valid = 1, lineRead = 0;
  
  do 
  {
    bytesRead = RS232_PollComport(COMPORT, (unsigned char *) &sCommBuf[sCommBufLen], BUFSZ - sCommBufLen - 1);
    if(bytesRead > 0) 
    {
      sCommBufLen += bytesRead;
      sCommBuf[sCommBufLen] = '\0';
      char *endOfLine = strchr(sCommBuf, '\n');
      if(endOfLine != NULL)
      {
        char *here = sCommBuf;
        for(int i = 0; i < data_Size && valid == 1; i++) 
        {
          while(isspace(*here) && here != endOfLine) here++;
          if(isdigit(*here)) data_Location[i] = strtol(here, &here, 10);
          else valid = 0;
        } // for
        if(valid == 1) 
        {
          while(isspace(*here) && here != endOfLine)
            here++;
          if(here != endOfLine)
            valid = 0;
          else
            lineRead = 1;
        }
        memmove(sCommBuf, endOfLine + 1, strlen(endOfLine + 1) + 1);
        sCommBufLen = strlen(sCommBuf);
      } // if(endOfLine
      else if(sCommBufLen >= BUFSZ - 1) 
      {
        sCommBufLen = 0;
        valid = 0;
      }
    } // if(bytesRead
  } 
  while(bytesRead > 0 && lineRead == 0 && valid == 1);

  if(bytesRead == 0)        return NODATA;
  else if(lineRead == 1)    return VALIDDATA;
  else                      return INVALIDDATA;
}