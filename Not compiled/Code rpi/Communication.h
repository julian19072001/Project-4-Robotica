#ifndef COMMUNICATION_H
#define COMMUNICATION_H
    #define BUFSZ 4096
    #define NODATA 0
    #define VALIDDATA 1
    #define INVALIDDATA -1

    int GetNewXMegaData(int comport, int *data_Location, int data_Size);
#endif