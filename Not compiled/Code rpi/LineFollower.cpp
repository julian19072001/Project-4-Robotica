#include "LineFollower.hpp"

BrickPi3 oLego;
static int lastError = 0;

void follow_Line(int* data_Location, int goal, float error_Gain, uint8_t rate_Of_Change, uint8_t motor_Left, uint8_t motor_Right, uint8_t speed)
{
    unsigned int linePos = ((data_Location[1] + data_Location[2]) - (data_Location[4] + data_Location[5]));
    int error = goal - linePos;
    int adjust = error*error_Gain + rate_Of_Change*(error - lastError);
 
    lastError = error;

    oLego.set_motor_dps(motor_Left, speed + adjust);
    oLego.set_motor_dps(motor_Right, speed - adjust);
}

void stop(uint8_t motor_Left, uint8_t motor_Right, uint8_t speed)
{
    oLego.set_motor_dps(motor_Left, speed * -2);
    oLego.set_motor_dps(motor_Right, speed * -2);
    usleep(20000);
    oLego.set_motor_dps(motor_Left, 0);
    oLego.set_motor_dps(motor_Right, 0);
}

int check_Line_Status(int* data_Location, uint8_t line_Samples, uint8_t min_Line_Change)
{
  static bool line;
  static uint16_t old_middle[line_Samples];
  if(old_middle[line_Samples-1] == 0) old_middle[line_Samples-1] = 2000;
  if(data_Location[3] > (old_middle[line_Samples-1] + (min_Line_Change*2.2)))                       line = false;
  else if(data_Location[3] < (old_middle[line_Samples-1] - (min_Line_Change*2.2)))                  line = true;
  else                                                                                              line = true;

  static bool left;
  static uint16_t old_left[line_Samples];
  if((data_Location[0] + data_Location[1]) < (old_left[line_Samples-1] - min_Line_Change))          left = true;
  else if((data_Location[0] + data_Location[1]) > (old_left[line_Samples-1] + min_Line_Change))     left = false;
  else                                                                                              left = false;


  static bool right;
  static uint16_t old_right[line_Samples];
  if((data_Location[5] + data_Location[6]) < (old_right[line_Samples-1] - min_Line_Change))         right = true;
  else if((data_Location[5] + data_Location[6]) > (old_right[line_Samples-1] + min_Line_Change))    right = false;
  else                                                                                              right = false;
  
  for(int i = line_Samples-1; i > 0; --i)
  {
    if(line == false)  old_middle[i]    = 2000;
    else               old_middle[i]    = old_middle[i-1];
    if(left == true)   old_left[i]    = 0;
    else               old_left[i]    = old_left[i-1];
    if(right == true)  old_right[i]   = 0;
    else               old_right[i]   = old_right[i-1];
  }
  old_middle[0] = data_Location[3];                     
  old_left[0]   = (data_Location[0] + data_Location[1]);
  old_right[0]  = (data_Location[5] + data_Location[6]);

  if(line == false) return NO_LINE;
  else if(right == true && left == true) return BOTH;
  else if(right == true && left == false) return RIGHT;
  else if(right == false && left == true) return LEFT;
  else return LINE;
}

int get_Road_Information(int* data_Location, uint8_t line_Samples, uint8_t min_Line_Change)
{
    static int line_left = 0;
    static int line_right = 0;
    static int turn_Detected = 0;

    static bool on_Line         = true;
    static bool cross_Detected  = false;

    int line_Status = check_Line_Status(data_Location, line_Samples, min_Line_Change);
    switch(line_Status)
    {
        case NO_LINE:
        on_Line = false;
        break;

        case BOTH:
        if(turn_Detected == 0)
        {   
            cross_Detected  = true;
            turn_Detected   = 1;
        }
        break;

        case LEFT:
        if(line_right > 0)
        {
            if(turn_Detected == 0)
            {
                cross_Detected  = true;
                turn_Detected = 1;
                line_right = 0;
            }
        }
        else 
        {
            if(turn_Detected == 0)
            {
              line_left++;
            }
        } 
        break;

        case RIGHT:
        if(line_left > 0)
        {
            if(turn_Detected == 0)
            {
              cross_Detected  = true;
              turn_Detected = 1;
              links = 0;
            }
        }
        else 
        {
            if(turn_Detected == 0)
            {
              line_right++;
            }
        }
        break;
    }

    if(line_right > 0)
    {
        if(line_right > line_Samples)
        {
            if(turn_Detected == 0)
            {
                line_right = 0;
                turn_Detected = 1;
                return OPTION_RIGHT;
            }
            else if(on_Line == false)
            {
                line_right = 0;
                return RIGHT_TURN;
            }
        }
        else line_right++;
    }

    if(line_left > 0)
    {
        if(line_left > line_Samples)
        {
            if(turn_Detected == 0)
            {
                line_left = 0;
                turn_Detected = 1;
                return OPTION_LEFT;
            }
            else if(on_Line == false)
            {
                line_right = 0;
                return LEFT_TURN;
            }
        }
        else line_left++;
    }

    if(!turn_Detected)
    {
        if(cross_Detected == true && on_Line == false)
        {
            cross_Detected = false;
            return SPLIT;
        }
        else if(cross_Detected == true && on_Line == true)
        {
            cross_Detected = false;
            return CROSS;
        }
    }

    if(on_Line == false)
    {
        if(!turn_Detected)
        {
            on_Line = true;
            return NO_LINE;
        }
    }

    if(turn_Detected > 0)
    {
        turn_Detected++;
        if(turn_Detected > 100) turn_Detected = 0;
    }
    
    return LINE;
}