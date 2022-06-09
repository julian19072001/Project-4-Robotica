#include "UI.hpp"

void Startup_UI(void)
{
    printf("\x1B[100T");
    
    printf("\x1B[1m");
    printf("\x1B[48;2;120;0;0m");
    printf("\x1B[38;2;255;255;255m");
    printf("\x1B[1;0H                    ");
    printf("\x1B[2;0H  ROBOTICA GROEP G  ");
    printf("\x1B[3;0H                    ");
    printf("\x1B[0m");
    
    printf("\x1B[1m");
    printf("\x1B[48;2;100;0;0m");
    printf("\x1B[38;2;255;255;255m");
    printf("\x1B[1;21H                                                                                                                                                                         ");
    printf("\x1B[2;21H                                                                                                                                                                         ");
    printf("\x1B[3;21H                                                                                                                                                                         ");
    printf("\x1B[0m");
    
    printf("\x1B[48;2;55;55;55m");
    printf("\x1B[38;2;255;255;255m");
    printf("\x1B[4;0H                                                                                                                                                       ");
    printf("\x1B[5;0H                                                                       Grid map                                                                        ");
    printf("\x1B[6;0H                                                                                                                                                       ");
    printf("\x1B[0m");

    printf("\x1B[48;2;100;100;100m");
    printf("\x1B[7;0H                                                                                                                                                       ");
    printf("\x1B[14;0H                                                                                                                                                       ");
    printf("\x1B[21;0H                                                                                                                                                       ");
    printf("\x1B[28;0H                                                                                                                                                       ");
    printf("\x1B[35;0H                                                                                                                                                       ");
    printf("\x1B[42;0H                                                                                                                                                       ");
    printf("\x1B[49;0H                                                                                                                                                       ");
    for(uint8_t y_pos = 7; y_pos < MOV_FEED_LENGTH + 8; y_pos++) 
    {
        printf("\x1B[%d;0H  ", y_pos);
        printf("\x1B[%d;14H  ", y_pos);
        printf("\x1B[%d;27H  ", y_pos);
        printf("\x1B[%d;40H  ", y_pos);
        printf("\x1B[%d;53H  ", y_pos);
        printf("\x1B[%d;66H  ", y_pos);
        printf("\x1B[%d;79H  ", y_pos);
        printf("\x1B[%d;92H  ", y_pos);
        printf("\x1B[%d;105H  ", y_pos);
        printf("\x1B[%d;118H  ", y_pos);
        printf("\x1B[%d;131H  ", y_pos);
        printf("\x1B[%d;144H  ", y_pos);
    }
    printf("\x1B[0m");
    printf("\x1B[48;2;45;45;45m");
    printf("\x1B[38;2;255;255;255m");
    printf("\x1B[4;146H                                            ");
    printf("\x1B[5;146H                Movement feed               ");
    printf("\x1B[6;146H                                            ");
    printf("\x1B[0m");
    
    printf("\x1B[48;2;15;15;15m");
    for(uint8_t y_pos = 7; y_pos < MOV_FEED_LENGTH + 8; y_pos++) printf("\x1B[%d;146H                             ", y_pos);
    printf("\x1B[0m"); 
    fflush(stdout);     
}

void Update_movement_feed(char *new_line)
{
    static char movement_feed[MOV_FEED_LENGTH + 3][MOV_FEED_WIDTH + 3];
    static uint8_t startup_switch = 0;

    if(startup_switch == 0) 
    {
        for(uint8_t i = 0; i < MOV_FEED_LENGTH + 1; i++) memset(movement_feed[i], 0, sizeof(movement_feed[i]));
        startup_switch = 1;
    }

    for(uint8_t arry_ptr = 0; arry_ptr < MOV_FEED_LENGTH + 1; arry_ptr++) strcpy(movement_feed[arry_ptr - 1], movement_feed[arry_ptr]);
    strcpy(movement_feed[MOV_FEED_LENGTH], new_line);

    printf("\x1B[48;2;15;15;15m");     

    for(uint8_t y_pos = 7; y_pos < MOV_FEED_LENGTH + 8; y_pos++) 
    {
        printf("\x1B[%d;146H %s ", y_pos, movement_feed[y_pos - 7]);
        if(strlen(movement_feed[y_pos - 7]) < MOV_FEED_WIDTH) for(uint8_t i = 0; i < MOV_FEED_WIDTH - strlen(movement_feed[y_pos - 7]); i++) printf(" ");
        printf(" ");
    }
    printf("\x1B[0m"); 
    fflush(stdout);  
}

void Update_grid(uint8_t pos, uint8_t colour, uint8_t x_max, uint8_t y_max)
{
    static uint8_t grid_sized = 0;

    uint8_t x_pos;
    uint8_t y_pos;

    pos--;

    printf("\x1B[0m");
    fflush(stdout);

    if(grid_sized == 0) {
        printf("\x1B[48;2;75;75;75m");
        for(x_pos = x_max; x_pos < 11; x_pos++) 
        {
            for(y_pos = 0; y_pos < 6; y_pos++) 
            {
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 8), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 9), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 10), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 11), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 12), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 13), ((x_pos * 13) + 3));
            }
        }

        for(y_pos = y_max; y_pos < 6; y_pos++) 
        {
            for(x_pos = 0; x_pos < 11; x_pos++) 
            {
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 8), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 9), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 10), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 11), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 12), ((x_pos * 13) + 3));
                printf("\x1B[%d;%dH           ", ((y_pos * 7) + 13), ((x_pos * 13) + 3));
            }
        }
        grid_sized = 1;
    }

    switch (colour % 10) 
    {
    case 0:
        printf("\x1B[48;2;130;130;130m");
        break;
    
    case 1:
        printf("\x1B[48;2;130;0;0m");
        break;
    
    case 2:
        printf("\x1B[48;2;130;65;0m");
        break;
    
    case 3:
        printf("\x1B[48;2;130;130;0m");
        break;
    
    case 4:
        printf("\x1B[48;2;0;130;0m");
        break;
    
    case 5:
        printf("\x1B[48;2;0;130;130m");
        break;
    
    case 6:
        printf("\x1B[48;2;0;0;130m");
        break;
    
    case 7:
        printf("\x1B[48;2;130;50;65m");
        break;
    
    case 8:
        printf("\x1B[48;2;100;0;130m");
        break;
    
    default:
        printf("\x1B[48;2;100;0;130m");
        break;
    }

    x_pos = pos % x_max;
    y_pos = pos / x_max;
    if(x_pos < 11 && y_pos < 6)
    {
        printf("\x1B[%d;%dH           ", ((y_pos * 7) + 8), ((x_pos * 13) + 3));
        printf("\x1B[%d;%dH           ", ((y_pos * 7) + 9), ((x_pos * 13) + 3));
        printf("\x1B[%d;%dH           ", ((y_pos * 7) + 10), ((x_pos * 13) + 3));
        printf("\x1B[%d;%dH           ", ((y_pos * 7) + 11), ((x_pos * 13) + 3));
        printf("\x1B[%d;%dH           ", ((y_pos * 7) + 12), ((x_pos * 13) + 3));
        printf("\x1B[%d;%dH           ", ((y_pos * 7) + 13), ((x_pos * 13) + 3));
    }

    printf("\x1B[0m");
    fflush(stdout);
}