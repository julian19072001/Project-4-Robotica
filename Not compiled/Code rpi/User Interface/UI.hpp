#ifndef UI_HPP
#define UI_HPP

    #define MOV_FEED_LENGTH 42
    #define MOV_FEED_WIDTH  41

    #include <stdint.h>
    #include <stdbool.h>
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>

    void Startup_UI(void);
    void Update_movement_feed(char *new_line);
    void Update_grid(uint8_t pos, uint8_t colour, uint8_t x_max, uint8_t y_max);

#endif
