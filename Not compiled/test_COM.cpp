#include <ctype.h>
#include "rs232.h"

#define BUFSZ 4096
#define COMPORT 1
#define NODATA 0
#define VALIDDATA 1
#define INVALIDDATA -1

int GetNewXMegaData(int *waarMoetenDeWaardesHeen, int hoeveelWaardesVerwachtenWe) 
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
        for(int i = 0; i < hoeveelWaardesVerwachtenWe && valid == 1; i++) 
        {
          while(isspace(*here) && here != endOfLine) here++;
          if(isdigit(*here)) waarMoetenDeWaardesHeen[i] = strtol(here, &here, 10);
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