#include "LineFollower.hpp"

void follow_Line(int* data_Location, int goal, float error_Gain, uint8_t rate_Of_Change, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed)
{
    static int lastError = 0;
    unsigned int linePos = ((data_Location[1] + data_Location[2]) - (data_Location[4] + data_Location[5]));
    int error = goal - linePos;
    int adjust = error*error_Gain + rate_Of_Change*(error - lastError);
 
    lastError = error;

    oLego.set_motor_dps(motor_Left, speed - adjust);
    oLego.set_motor_dps(motor_Right, speed + adjust);
}

int turn_Right(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change)
{
    int origin_Line = check_Line_Status(data_Location, min_Line_Change);
    if(origin_Line == RIGHT)
    {
        oLego.set_motor_dps(motor_Left, 0);
        oLego.set_motor_dps(motor_Right, 0);
        return STRAIGHT;
    }
    else
    {
        oLego.set_motor_dps(motor_Left, speed);
        oLego.set_motor_dps(motor_Right, speed * -1);
        return TURNING_RIGHT;
    }
}

int turn_Left(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change)
{
    int origin_Line = check_Line_Status(data_Location, min_Line_Change);
    if(origin_Line == LEFT)
    {
        oLego.set_motor_dps(motor_Left, 0);
        oLego.set_motor_dps(motor_Right, 0);
        return STRAIGHT;
    }
    else
    {
        oLego.set_motor_dps(motor_Left, speed * -1);
        oLego.set_motor_dps(motor_Right, speed);
        return TURNING_LEFT;
    }
}

int turn_180(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change)
{
    int origin_Line = check_Line_Status(data_Location, min_Line_Change);
    static bool first_Line_Past = false;
    if(origin_Line == LEFT && first_Line_Past == true || origin_Line == RIGHT && first_Line_Past == true)
    {
        oLego.set_motor_dps(motor_Left, 0);
        oLego.set_motor_dps(motor_Right, 0);
        first_Line_Past = false;
        return STRAIGHT;
    }
    else if(first_Line_Past == false && origin_Line == LEFT)
    {
        oLego.set_motor_dps(motor_Left, speed * -1);
        oLego.set_motor_dps(motor_Right, speed);
        first_Line_Past = true;
        return TURNING_180;
    }
    else
    {
        oLego.set_motor_dps(motor_Left, speed * -1);
        oLego.set_motor_dps(motor_Right, speed);
        return TURNING_180;
    }
}

void stop(uint8_t motor_Left, uint8_t motor_Right, uint16_t speed)
{
    oLego.set_motor_dps(motor_Left, speed * -4);
    oLego.set_motor_dps(motor_Right, speed * -4);
    usleep(50000);
    oLego.set_motor_dps(motor_Left, 0);
    oLego.set_motor_dps(motor_Right, 0);
}

int check_Line_Status(int* data_Location, uint16_t min_Line_Change)
{
  static bool line;
  static uint16_t old_middle[LINE_SAMPLES];
  if(old_middle[LINE_SAMPLES-1] == 0) old_middle[LINE_SAMPLES-1] = 2000;
  if(data_Location[3] > (old_middle[LINE_SAMPLES-1] + min_Line_Change))                       line = false;
  else if(data_Location[3] < (old_middle[LINE_SAMPLES-1] - min_Line_Change))                  line = true;
  else                                                                                              line = true;

  static bool left;
  static uint16_t old_left[LINE_SAMPLES];
  if((data_Location[0] + data_Location[1]) < (old_left[LINE_SAMPLES-1] - min_Line_Change))          left = true;
  else if((data_Location[0] + data_Location[1]) > (old_left[LINE_SAMPLES-1] + min_Line_Change))     left = false;
  else                                                                                              left = false;


  static bool right;
  static uint16_t old_right[LINE_SAMPLES];
  if((data_Location[5] + data_Location[6]) < (old_right[LINE_SAMPLES-1] - min_Line_Change))         right = true;
  else if((data_Location[5] + data_Location[6]) > (old_right[LINE_SAMPLES-1] + min_Line_Change))    right = false;
  else                                                                                              right = false;
  
  for(int i = LINE_SAMPLES-1; i > 0; --i)
  {
    if(line == false)  old_middle[i]  = 2000;
    else               old_middle[i]  = old_middle[i-1];
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

int get_Road_Information(int* data_Location, uint16_t min_Line_Change)
{
    static int line_left = 0;
    static int line_right = 0;
    static int turn_Detected = 0;

    static bool on_Line         = true;
    static bool cross_Detected  = false;

    int line_Status = check_Line_Status(data_Location, min_Line_Change);
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
              line_left = 0;
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
        if(line_right > 100)
        {
            if(!turn_Detected && on_Line == true)
            {
                line_right = 0;
                turn_Detected = 1;
                return OPTION_RIGHT;
            }
            else
            {
                line_right = 0;
                return RIGHT_TURN; 
            }
        }
        else line_right++;
    }

    if(line_left > 0)
    {
        if(line_left > 100)
        {
            if(!turn_Detected && on_Line == true)
            {
                line_left = 0;
                turn_Detected = 1;
                return OPTION_LEFT;
            }
            else
            {
                line_left = 0;
                return LEFT_TURN; 
            }
        }
        else line_left++;
    }

    if(!turn_Detected && cross_Detected == true)
    {
        if(on_Line == false)
        {
            cross_Detected = false;
            return SPLIT;
        }
        else 
        {
            cross_Detected = false;
            return CROSS;
        }
    }

    if(on_Line == false && !turn_Detected && !line_left && !line_right)
    {
            on_Line = true;
            return NO_LINE;
    }

    if(turn_Detected > 0)
    {
        turn_Detected++;
        if(turn_Detected > 100) turn_Detected = 0;
    }
    
    return LINE;
}