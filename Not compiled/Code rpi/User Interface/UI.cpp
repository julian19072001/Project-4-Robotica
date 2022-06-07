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
    printf("\x1B[1;21H                                                                                                ");
    printf("\x1B[2;21H                                                                                                ");
    printf("\x1B[3;21H                                                                                                ");
    printf("\x1B[0m");
    
    printf("\x1B[48;2;55;55;55m");
    printf("\x1B[38;2;255;255;255m");
    printf("\x1B[4;0H                                                                             ");
    printf("\x1B[5;0H                                  Grid map                                   ");
    printf("\x1B[6;0H                                                                             ");
    printf("\x1B[0m");

    printf("\x1B[48;2;100;100;100m");
    printf("\x1B[7;0H                                                                            ");
    printf("\x1B[15;0H                                                                            ");
    printf("\x1B[23;0H                                                                            ");
    printf("\x1B[31;0H                                                                            ");
    printf("\x1B[39;0H                                                                            ");
    printf("\x1B[47;0H                                                                            ");
    for(uint8_t y_pos = 7; y_pos < MOV_FEED_LENGTH + 8; y_pos++) {
        printf("\x1B[%d;0H  ", y_pos);
        printf("\x1B[%d;16H  ", y_pos);
        printf("\x1B[%d;31H  ", y_pos);
        printf("\x1B[%d;46H  ", y_pos);
        printf("\x1B[%d;61H  ", y_pos);
        printf("\x1B[%d;76H  ", y_pos);
    };
    printf("\x1B[0m");
    
    printf("\x1B[48;2;45;45;45m");
    printf("\x1B[38;2;255;255;255m");
    printf("\x1B[4;78H                                       ");
    printf("\x1B[5;78H             Movement feed             ");
    printf("\x1B[6;78H                                       ");
    printf("\x1B[0m");
    
    printf("\x1B[48;2;15;15;15m");
    for(uint8_t y_pos = 7; y_pos < MOV_FEED_LENGTH + 8; y_pos++) {
        printf("\x1B[%d;78H                              ", y_pos);
    };
    printf("\x1B[0m"); 
    fflush(stdout);     
}

void Update_movement_feed(char *new_line)
{
    static char movement_feed[MOV_FEED_LENGTH + 3][MOV_FEED_WIDTH + 3];
    static uint8_t startup_switch = 0;

    if(startup_switch == 0) {
        for(uint8_t i = 0; i < MOV_FEED_LENGTH + 1; i++) {
            memset(movement_feed[i], 0, sizeof(movement_feed[i]));
        }; startup_switch = 1;
    }

    for(uint8_t arry_ptr = 0; arry_ptr < MOV_FEED_LENGTH + 1; arry_ptr++) {
        strcpy(movement_feed[arry_ptr - 1], movement_feed[arry_ptr]);
    }; strcpy(movement_feed[MOV_FEED_LENGTH], new_line);

    printf("\x1B[48;2;15;15;15m");   

    for(uint8_t y_pos = 7; y_pos < MOV_FEED_LENGTH + 8; y_pos++) {
        printf("\x1B[%d;78H %s ", y_pos, movement_feed[y_pos - 7]);

        if(strlen(movement_feed[y_pos - 7]) < MOV_FEED_WIDTH) {
            for(uint8_t i = 0; i < MOV_FEED_WIDTH - strlen(movement_feed[y_pos - 7]); i++) {
                printf(" ");
            };
        }; 
        printf(" ");
    };
    printf("\x1B[0m");  
    fflush(stdout);    
}

void Update_grid(uint8_t pos, uint8_t colour)
{
    uint8_t x_pos;
    uint8_t y_pos;

    switch (colour) {
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
        break;
    }

    x_pos = pos % 5;
    y_pos = pos / 5;

    printf("\x1B[%d;%dH             ", ((y_pos * 8) + 8), ((x_pos * 15) + 3));
    printf("\x1B[%d;%dH             ", ((y_pos * 8) + 9), ((x_pos * 15) + 3));
    printf("\x1B[%d;%dH             ", ((y_pos * 8) + 10), ((x_pos * 15) + 3));
    printf("\x1B[%d;%dH             ", ((y_pos * 8) + 11), ((x_pos * 15) + 3));
    printf("\x1B[%d;%dH             ", ((y_pos * 8) + 12), ((x_pos * 15) + 3));
    printf("\x1B[%d;%dH             ", ((y_pos * 8) + 13), ((x_pos * 15) + 3));
    printf("\x1B[%d;%dH             ", ((y_pos * 8) + 14), ((x_pos * 15) + 3));
    printf("\x1B[0m");
    fflush(stdout);
};