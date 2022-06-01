#include "Communication.h"

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