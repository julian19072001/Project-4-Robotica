//g++ -Wall -o COM test_COM.cpp ~/hva_libraries/legorpi/*.cpp -I/home/piuser/hva_libraries/legorpi ~/hva_libraries/legorpi/*.c
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <rs232.h>

#define BUFSZ 4096
#define COMPORT 24
#define NODATA 0
#define VALIDDATA 1
#define INVALIDDATA -1
#define AANTAL_WAARDES 7

//Method to catch the CTRL+C signal.
void exit_signal_handler(int signo);
//Get new data form the xMega
int GetNewXMegaData(int *data_Location, int data_Size);
// Start of the program
int main(int nArgc, char* aArgv[]) {
  // Register the exit function for Ctrl+C
  signal(SIGINT, exit_signal_handler);
  
  int waarde[AANTAL_WAARDES];
  int commIsOpen = 0;

  commIsOpen = !RS232_OpenComport(COMPORT, 115200, "8N1", 0);
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
        printf("Succes! Waardes zijn:");
        for(int i = 0; i < AANTAL_WAARDES; i++) printf(" %d", waarde[i]);
        printf("\n");
      } 
      else if(regelResult == INVALIDDATA) printf("Error\n Data niet goed ontvangen!\n"); //en als regelResult == 0, dan was er nog geen nieuwe regel binnen
      
    }
    //sleep(1);
  };
};

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo) {

  if (signo == SIGINT) {
    // Reset everything so there are no run-away motors
    printf("\nThe LEGO RPi example has stopped\n\n");
    exit(-2);
  }
};

int GetNewXMegaData(int *data_Location, int data_Size) {
  static char sCommBuf[BUFSZ];
  static int sCommBufLen = 0;
  int bytesRead = 0, valid = 1, lineRead = 0;
  
  do {
    bytesRead = RS232_PollComport(COMPORT, (unsigned char *) &sCommBuf[sCommBufLen], 1);
    if(bytesRead > 0) {
      sCommBufLen += bytesRead;
      sCommBuf[sCommBufLen] = '\0';
      if(sCommBuf[sCommBufLen - 1] == '\n') {
        char *here = sCommBuf;
        for(int i = 0; i < data_Size && valid == 1; i++) {
          while(isspace(*here))
            here++;
          if(isdigit(*here))
            data_Location[i] = strtol(here, &here, 10);
          else
            valid = 0;
        } // for
        if(valid == 1) {
          while(isspace(*here))
            here++;
          if(*here != '\0')
            valid = 0;
          else
            lineRead = 1;
        }
        sCommBufLen = 0;
      } // if(sCommBuf
      else if(sCommBufLen >= BUFSZ - 1) {
        sCommBufLen = 0;
        valid = 0;
      }
    } // if(bytesRead
  } while(bytesRead > 0 && lineRead == 0 && valid == 1);

  if(bytesRead == 0)
    return NODATA;
  else if(lineRead == 1)
    return VALIDDATA;
  else
    return INVALIDDATA;
}