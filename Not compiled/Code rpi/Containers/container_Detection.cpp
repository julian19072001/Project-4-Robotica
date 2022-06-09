#include "container_Detection.hpp"

#define IS_THERE        0
#define IS_NOT_THERE    1

static Container_t containers[MAX_NUMBER_OF_CONTAINERS];

static uint8_t number_Scanned_Containers = 0;

void save_Container(int16_t x_Pos, int16_t y_Pos, int comPort, char side);
int check_If_There(int16_t x_Pos, int16_t y_Pos);

void swap(int16_t *pos1, int16_t *pos2)
{
    int16_t temp = *pos1;
    *pos1 = *pos2;
    *pos2 = temp;
}

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
    if(left_Detected < (MIN_DETECTED)) return;
    left_Detected = 0;
    usleep(1350);
    if(y_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, comPort, 'L');
    }
    else if(y_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, comPort, 'L');
    }
    else if(x_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, comPort, 'L');
    }
    else if(x_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, comPort, 'L');
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
    if(right_Detected < MIN_DETECTED) return;
    right_Detected = 0;
    usleep(200);
    if(y_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, comPort, 'R');
    }
    else if(y_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, comPort, 'R');
    }
    else if(x_Direction_Modifier == 1)
    {
        int16_t container_X_Pos = x_Pos;
        int16_t container_Y_Pos = y_Pos - 1;
        save_Container(container_X_Pos, container_Y_Pos, comPort, 'R');
    }
    else if(x_Direction_Modifier == -1)
    {
        int16_t container_X_Pos = x_Pos - 1;
        int16_t container_Y_Pos = y_Pos;
        save_Container(container_X_Pos, container_Y_Pos, comPort, 'R');
    }
}

void print_Found_Containers(int16_t x_Min, int16_t y_Min, int16_t x_Max, int16_t y_Max)
{
    char feed_entry[MOV_FEED_WIDTH];

    Update_movement_feed("");
    Update_movement_feed("Gevonden containers");

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
        memset(feed_entry, 0, sizeof(feed_entry));
        sprintf(feed_entry, "%s%d", "Container: ", i + 1);
        Update_movement_feed(feed_entry);

        memset(feed_entry, 0, sizeof(feed_entry));
        sprintf(feed_entry, "%s%d", "Positie: ", containers[i].position);
        Update_movement_feed(feed_entry);
        
        //printf("Coördinaat: (%d,%d)\n", containers[i].x_Pos, containers[i].y_Pos);
        switch(containers[i].color % 10)
        {
            case 0:
            Update_movement_feed("Geen kleur");
            Update_movement_feed("");
            break;

            case 1:
            Update_movement_feed("Kleur: Rood");
            Update_movement_feed("");
            break;
            
            case 2:
            Update_movement_feed("Kleur: Oranje");
            Update_movement_feed("");
            break;

            case 3:
            Update_movement_feed("Kleur: Geel");
            Update_movement_feed("");
            break;

            case 4:
            Update_movement_feed("Kleur: Groen");
            Update_movement_feed("");
            break;

            case 5:
            Update_movement_feed("Kleur: Licht blauw");
            Update_movement_feed("");
            break;

            case 6:
            Update_movement_feed("Kleur: Blauw");
            Update_movement_feed("");
            break;

            case 7:
            Update_movement_feed("Kleur: Roze");
            Update_movement_feed("");
            break;

            case 8:
            Update_movement_feed("Kleur: Paars");
            Update_movement_feed("");
            break;

            default:
            memset(feed_entry, 0, sizeof(feed_entry));
            sprintf(feed_entry, "%s%d", "Kleur: ", containers[i].color);
            Update_movement_feed(feed_entry);
            Update_movement_feed("");
            break;

        }

        Update_grid(containers[i].position, containers[i].color, x_Length, y_Length);
    }
}


void sort_Containers()
{
    for (int i = 0; i < number_Scanned_Containers; i++) 
    {     
        for (int j = i+1; j < number_Scanned_Containers; j++) 
        {     
            if(containers[i].position > containers[j].position) 
            {    
                swap(&containers[i].x_Pos, &containers[j].x_Pos);
                swap(&containers[i].y_Pos, &containers[j].y_Pos);
                swap(&containers[i].color, &containers[j].color);
                swap(&containers[i].position, &containers[j].position);    
           }     
        }     
    } 
}

void save_Container(int16_t x_Pos, int16_t y_Pos, int comPort, char side)
{
    if(check_If_There(x_Pos, y_Pos) == IS_THERE && number_Scanned_Containers) return;
    else
    {
        RS232_SendByte(comPort, side);
        Update_movement_feed("Container gevonden");
        containers[number_Scanned_Containers].x_Pos = x_Pos;
        containers[number_Scanned_Containers].y_Pos = y_Pos;
        
        int color[1];
        color[0] = 0;

        static struct timeval current_time;
        gettimeofday(&current_time, NULL);
        int64_t start_Time = current_time.tv_usec;
    
        while(GetNewXMegaData(comPort, (int*)color, 1) != VALIDDATA && current_time.tv_usec < start_Time + WAIT_FOR_COLOR) gettimeofday(&current_time, NULL);
        containers[number_Scanned_Containers].color = color[0];
        number_Scanned_Containers++;
        return;
    } 
}

int check_If_There(int16_t x_Pos, int16_t y_Pos)
{
    for(int i = 0; i < number_Scanned_Containers; i++) if(x_Pos == containers[i].x_Pos && y_Pos == containers[i].y_Pos) return IS_THERE;
    return IS_NOT_THERE;
}

int get_Number_Of_Containers()
{
    return number_Scanned_Containers;
}