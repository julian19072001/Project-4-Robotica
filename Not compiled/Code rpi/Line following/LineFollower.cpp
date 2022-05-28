#include "LineFollower.hpp"

BrickPi3 oLego;

void drive_Straight(uint8_t motor_Left, uint8_t motor_Right, uint16_t speed)
{
    oLego.set_motor_dps(motor_Left, speed);
    oLego.set_motor_dps(motor_Right, speed);
}

void follow_Line(int* data_Location, int goal, float error_Gain, uint8_t rate_Of_Change, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change, uint16_t mid_Line_Change)
{
    static int lastError = 0;
    static int old_Left = 0;
    static int old_Right = 0;
    static int side_Line = 0;
    int adjust = 0;

    if(side_Line > 0) side_Line++;
    if(side_Line > 33) side_Line = 0;
    
    if(data_Location[0] < (old_Left - (min_Line_Change / 3.0)) || data_Location[6] < (old_Right - (min_Line_Change / 3.0))) side_Line = 1;
    
    if(!side_Line)
    {
        unsigned int linePos = (data_Location[1] + data_Location[2]) - (data_Location[4] + data_Location[5]);
        int error = goal - linePos;
        adjust = error*error_Gain + rate_Of_Change*(error - lastError);
 
        lastError = error;
    } 

    old_Left = data_Location[0];
    old_Right = data_Location[6];
      
    oLego.set_motor_dps(motor_Left, speed + adjust);
    oLego.set_motor_dps(motor_Right, speed - adjust);
}

int turn_Right(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change, uint16_t mid_Line_Change, uint16_t wait_Samples, uint16_t line_Samples)
{
    static int count = 0;
    if(count % 3 == 0)
    {
        static int right_Detected = 0;
        int origin_Line = check_Line_Status(data_Location, (min_Line_Change/2.5) - 50, mid_Line_Change, line_Samples);
        
        if(origin_Line == RIGHT)
        {   
            right_Detected++;
        }

        if(right_Detected > 1 && (count > (wait_Samples * 3)))
        {
            oLego.set_motor_dps(motor_Left, 0);
            oLego.set_motor_dps(motor_Right, 0);
            count = 0;
            right_Detected = 0;
            return STRAIGHT;
        }
        else
        {
            oLego.set_motor_dps(motor_Left, speed);
            oLego.set_motor_dps(motor_Right, -speed);
            count++;
            return TURNING_RIGHT;
        }
    }
    count++;
    return TURNING_RIGHT;
}

int turn_Left(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change, uint16_t mid_Line_Change, uint16_t wait_Samples, uint16_t line_Samples)
{
    static int count = 0;
    if(count % 3 == 0)
    {
        static int left_Detected = 0;
        int origin_Line = check_Line_Status(data_Location, (min_Line_Change/2.5) - 50, mid_Line_Change, line_Samples);
        
        if(origin_Line == LEFT)
        {   
            left_Detected++;
        }

        if(left_Detected > 1 && (count > (wait_Samples * 3)))
        {
            oLego.set_motor_dps(motor_Left, 0);
            oLego.set_motor_dps(motor_Right, 0);
            count = 0;
            left_Detected = 0;
            return STRAIGHT;
        }
        else
        {
            oLego.set_motor_dps(motor_Left, -speed);
            oLego.set_motor_dps(motor_Right, speed);
            count++;
            return TURNING_LEFT;
        }
    }
    count++;
    return TURNING_LEFT;
}

int turn_180(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change, uint16_t mid_Line_Change, uint16_t wait_Samples, uint16_t line_Samples)
{   
    static bool turned_90 = false;
    static int count = 0;
    if(count % 3 == 0)
    {
        static int side_Line_Detected = 0;
        int origin_Line = check_Line_Status(data_Location, (min_Line_Change/2.0) - 50, mid_Line_Change, line_Samples);
        
        if(origin_Line == LEFT) side_Line_Detected++; 

        if(side_Line_Detected > 1 && (count > (wait_Samples * 3)))
        {
            if(turned_90 == true)
            {
                oLego.set_motor_dps(motor_Left, 0);
                oLego.set_motor_dps(motor_Right, 0);
                count = 0;
                side_Line_Detected = 0;
                turned_90 = false;
                return STRAIGHT;
            }
            else
            {
                oLego.set_motor_dps(motor_Left, -speed);
                oLego.set_motor_dps(motor_Right, speed);
                count++;
                side_Line_Detected = 0;
                turned_90 = true;
                return TURNING_180; 
            }
        }
        else
        {
            oLego.set_motor_dps(motor_Left, -speed);
            oLego.set_motor_dps(motor_Right, speed);
            count++;
            return TURNING_180;
        }
    }
    count++;
    return TURNING_180;
}

int turn_180_Right(int* data_Location, uint8_t motor_Left, uint8_t motor_Right, uint16_t speed, uint16_t min_Line_Change, uint16_t mid_Line_Change, uint16_t wait_Samples, uint16_t line_Samples)
{   
    static bool turned_90 = false;
    static int count = 0;
    if(count % 3 == 0)
    {
        static int side_Line_Detected = 0;
        int origin_Line = check_Line_Status(data_Location, (min_Line_Change/2.0) - 50, mid_Line_Change, line_Samples);
        
        if(origin_Line == RIGHT) side_Line_Detected++; 

        if(side_Line_Detected > 1 && (count > (wait_Samples * 3)))
        {
            if(turned_90 == true)
            {
                oLego.set_motor_dps(motor_Left, 0);
                oLego.set_motor_dps(motor_Right, 0);
                count = 0;
                side_Line_Detected = 0;
                turned_90 = false;
                return STRAIGHT;
            }
            else
            {
                oLego.set_motor_dps(motor_Left, speed);
                oLego.set_motor_dps(motor_Right, -speed);
                count++;
                side_Line_Detected = 0;
                turned_90 = true;
                return TURNING_180_RIGHT; 
            }
        }
        else
        {
            oLego.set_motor_dps(motor_Left, speed);
            oLego.set_motor_dps(motor_Right, -speed);
            count++;
            return TURNING_180_RIGHT;
        }
    }
    count++;
    return TURNING_180_RIGHT;
}

void reset_Lego()
{
    oLego.reset_all();
}

int check_Line_Status(int* data_Location, uint16_t min_Line_Change, uint16_t mid_Line_Change, uint16_t line_Samples)
{
  static bool line;
  static uint16_t old_middle[line_Samples];
  if(old_middle[line_Samples-1] == 0) old_middle[line_Samples-1] = MAX_ADC_RETURN;
  if(data_Location[3] > (old_middle[line_Samples-1] + (mid_Line_Change)))           line = false;
  else if(data_Location[3] < (old_middle[line_Samples-1] - (mid_Line_Change)))      line = true;
  else                                                                              line = true;

  static bool left;
  static uint16_t old_left[line_Samples];
  if((data_Location[0]) < (old_left[line_Samples-1] - min_Line_Change))             left = true;
  else if((data_Location[0] ) > (old_left[line_Samples-1] + min_Line_Change))       left = false;
  else                                                                              left = false;


  static bool right;
  static uint16_t old_right[line_Samples];
  if((data_Location[6]) < (old_right[line_Samples-1] - min_Line_Change))            right = true;
  else if((data_Location[6]) > (old_right[line_Samples-1] + min_Line_Change))       right = false;
  else                                                                              right = false;
  
  for(int i = line_Samples-1; i > 0; --i)
  {
    if(line == false)  old_middle[i]  = MAX_ADC_RETURN;
    else               old_middle[i]  = old_middle[i-1];
    if(left == true)   old_left[i]    = MIN_ADC_RETURN;
    else               old_left[i]    = old_left[i-1];
    if(right == true)  old_right[i]   = MIN_ADC_RETURN;
    else               old_right[i]   = old_right[i-1];
  }
  old_middle[0] = data_Location[3];                     
  old_left[0]   = (data_Location[0]);
  old_right[0]  = (data_Location[6]);

  if(line == false) return NO_LINE;
  else if(right == true && left == true) return BOTH;
  else if(right == true && left == false) return RIGHT;
  else if(right == false && left == true) return LEFT;
  else return LINE;
}

int get_Road_Information(int* data_Location, uint16_t min_Line_Change, uint16_t mid_Line_Change, uint16_t wait_Samples, uint16_t line_Samples)
{
    static int line_left = 0;
    static int line_right = 0;
    static int turn_Detected = 0;

    static bool on_Line         = true;
    static bool cross_Detected  = false;

    int line_Status = check_Line_Status(data_Location, min_Line_Change, mid_Line_Change, line_Samples);
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
        if(line_right > wait_Samples)
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
        if(line_left > wait_Samples)
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
        if(turn_Detected > wait_Samples) turn_Detected = 0;
    }
    
    return LINE;
}