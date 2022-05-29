#include "container_Detection.hpp"

#define IS_THERE        0
#define IS_NOT_THERE    1

static Container_t containers[MAX_NUMBER_OF_CONTAINERS];

static uint8_t number_Scanned_Containers = 0;

void save_Container(int16_t x_Pos, int16_t y_Pos, int comPort);
int check_If_There(int16_t x_Pos, int16_t y_Pos);

void check_Container_Left(uint8_t left_Distance, uint8_t max_Distance, int comPort, int8_t x_Direction_Modifier, int16_t x_Pos, int8_t y_Direction_Modifier, int16_t y_Pos)
{
    static int left_Detected = 0;
    if(number_Scanned_Containers > MAX_NUMBER_OF_CONTAINERS) return;
    if(!left_Distance || ((left_Distance > max_Distance) && left_Detected == 0))
    {
        left_Detected = 0;
        return;
    }
    left_Detected++;
    /*if(left_Detected == 5)
    {
        RS232_SendByte(comPort, 'S');
    }*/
    if(left_Detected < MIN_DETECTED) return;
    left_Detected = 0;
    if(y_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, comPort);
    }
    else if(y_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, comPort);
    }
    else if(x_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, comPort);
    }
    else if(x_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, comPort);
    }
}

void check_Container_Right(uint8_t right_Distance, uint8_t max_Distance, int comPort, int8_t x_Direction_Modifier, int16_t x_Pos, int8_t y_Direction_Modifier, int16_t y_Pos)
{
    static int right_Detected = 0;
    if(number_Scanned_Containers > MAX_NUMBER_OF_CONTAINERS) return;
    if(!right_Distance || ((right_Distance > max_Distance) && right_Detected == 0))
    {
        right_Detected = 0;
        return;
    }
    right_Detected++;
    /*if(right_Detected == 5)
    {
        RS232_SendByte(comPort, 'S');
    }*/
    if(right_Detected < MIN_DETECTED) return;
    right_Detected = 0;
    if(y_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, comPort);
    }
    else if(y_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, comPort);
    }
    else if(x_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, comPort);
    }
    else if(x_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, comPort);
    }
}

//sort the containers by position number and print them in the terminal
void print_Found_Containers(int16_t x_Min, int16_t y_Min, int16_t x_Max, int16_t y_Max)
{
    printf("\n\n\n\n\n\n\nGevonden containers:\n");
    int x_Length = x_Max - x_Min;
    int y_Length = y_Max - y_Min;
    for(int i = 0; i < number_Scanned_Containers; i++)
    {
        int real_X_Pos = (containers[i].x_Pos - x_Min);
        int real_Y_Pos = (containers[i].y_Pos - y_Min);
        containers[i].position = ((y_Length - real_Y_Pos - 1) * x_Length) + real_X_Pos + 1;
    }
    sort_Containers();
    for(int i = 0; i < number_Scanned_Containers; i++)
    {
        printf("Container: %d\n", i + 1);
        printf("Positie: %2d\n", containers[i].position);
        printf("Coördinaat: (%d,%d)\n\n", containers[i].x_Pos, containers[i].y_Pos);
        //printf("Kleur: %d\n\n", containers[i].color);
    }
}


//function to perform sorting
void sort_Containers();
{
    int min_idx;
 
    // One by one move boundary of unsorted subarray
    for (int i = 0; i < number_Scanned_Containers - 1; i++) 
    {
        //find the minimum element in unsorted array
        min_idx = i;
        for (int j = i + 1; j < number_Scanned_Containers; j++) if (containers[j].position < containers[min_idx].position) min_idx = j;
 
        //swap the found minimum element
        //with the first element
        swap(&containers[min_idx].x_Pos, &containers[j].x_Pos);
        swap(&containers[min_idx].y_Pos, &containers[j].y_Pos);
        swap(&containers[min_idx].color, &containers[j].color);
        swap(&containers[min_idx].position, &containers[j].position);
    }
}

//save the found container
void save_Container(int16_t x_Pos, int16_t y_Pos, int comPort)
{
    if(check_If_There(x_Pos, y_Pos) == IS_THERE && number_Scanned_Containers) return;
    else
    {
        containers[number_Scanned_Containers].x_Pos = x_Pos;
        containers[number_Scanned_Containers].y_Pos = y_Pos;
        
        /*int color[1];

        static struct timeval current_time;
        gettimeofday(&current_time, NULL);
        int64_t start_Time = current_time.tv_usec;
    
        while(GetNewXMegaData(comPort, (int*)color, 1) != VALIDDATA || current_time.tv_usec > start_Time + WAIT_FOR_COLOR);
        containers[number_Scanned_Containers].color = color[0];*/
        number_Scanned_Containers++;
        return;
    } 
}

//function to check if the container is already saved
int check_If_There(int16_t x_Pos, int16_t y_Pos)
{
    for(int i = 0; i < number_Scanned_Containers; i++)
    {
        if(x_Pos == containers[i].x_Pos && y_Pos == containers[i].y_Pos) return IS_THERE;
    }
    return IS_NOT_THERE;
}