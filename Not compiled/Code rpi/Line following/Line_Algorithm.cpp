#include "Line_Algorithm.hpp"

algorithm::algorithm()
{
    reset_Variabels();
    just_Turned = 1;

    goal_c = 0;
    rate_Of_Change_c = 2;
    motor_Left_c = PORT6_MA;
    motor_Right_c = PORT5_MB;
    driving_Speed_c = -100;
    turning_Speed_c = -50;
    line_Samples_c = 20;
    wait_Samples_c = 100;
    min_Side_Line_Change_c = 300;
    min_Mid_Line_Change_c = 1000;
    error_Gain_c = 0.018;
}

void algorithm::reset_Variabels()
{
    x_Pos = 0;
    x_Min = 0;
    x_Max = 0;
    x_Direction_Modifier = 0;

    y_Pos = 0;
    y_Min = 0;
    y_Max = 0;
    y_Direction_Modifier = 0;

    side_Scanned = false;
    reached_Y_Min = false;

    program_State_c = SEARCH_LINE;
    driving_State = STRAIGHT;
}

int algorithm::search_Line()
{
    if(just_Turned > 0) just_Turned++;
    if(just_Turned > wait_Samples_c*4) just_Turned = 0;

    road = get_Road_Information(data_Location_c, min_Side_Line_Change_c, min_Mid_Line_Change_c, wait_Samples_c, line_Samples_c);
    switch(road)
    {
        case LINE:
        if(!just_Turned) follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
        else follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
        break;

        case CROSS: 
        Update_movement_feed("Kruising");
        y_Pos = 1;
        y_Direction_Modifier = 1;
        program_State_c = DRIVE_OVER_GRID;
        break;

        case OPTION_LEFT:
        Update_movement_feed("Splitsing links");
        y_Pos = 1;
        y_Direction_Modifier = 1;
        program_State_c = DRIVE_OVER_GRID;
        break;

        case OPTION_RIGHT:
        Update_movement_feed("Splitsing rechts");
        y_Pos = 1;
        y_Direction_Modifier = 1;
        program_State_c = DRIVE_OVER_GRID;
        break;

        case SPLIT:
        Update_movement_feed("Splitsing");
        y_Pos = 1;
        x_Direction_Modifier = 1;
        y_Max = 1;
        program_State_c = DRIVE_OVER_GRID;
        driving_State = TURNING_RIGHT;
        break;

        case LEFT_TURN:
        Update_movement_feed("Bocht links");
        y_Pos = 1;
        x_Direction_Modifier = -1;
        side_Scanned = true;
        program_State_c = DRIVE_OVER_GRID;
        driving_State = TURNING_LEFT;
        break;

        case RIGHT_TURN:
        Update_movement_feed("Bocht rechts");
        y_Pos = 1;
        x_Direction_Modifier = 1;
        program_State_c = DRIVE_OVER_GRID;
        driving_State = TURNING_RIGHT;
        break;

        case NO_LINE:
        Update_movement_feed("Error met detecteren van de lijn.");
        break;
    }
    return program_State_c;
}

int algorithm::drive_Over_Grid()
{
    if(just_Turned > 0) just_Turned++;
    if(just_Turned > wait_Samples_c*4) just_Turned = 0;

    switch(driving_State)
    {
        case STRAIGHT:
        road = get_Road_Information(data_Location_c, min_Side_Line_Change_c, min_Mid_Line_Change_c, wait_Samples_c, line_Samples_c);
        switch(road)
        {
            case LINE:
            if(!just_Turned) follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
            else follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
            break;

            case CROSS:
            Update_movement_feed("Kruising");
            x_Pos += (1 * x_Direction_Modifier);
            y_Pos += (1 * y_Direction_Modifier);
            follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
            break;

            case OPTION_LEFT:
            if(!just_Turned)
            {
                Update_movement_feed("Splitsing links");
                x_Pos += (1 * x_Direction_Modifier);
                y_Pos += (1 * y_Direction_Modifier);
                if(!y_Direction_Modifier && x_Pos != 0 && (x_Pos % 2 == 0 && (side_Scanned == false || (side_Scanned == true && x_Pos < 0))))
                {
                    driving_State = TURNING_LEFT;
                    if(x_Direction_Modifier == 1)
                    {
                        x_Direction_Modifier = 0;
                        y_Direction_Modifier = 1;
                    }
                    else
                    {
                        x_Direction_Modifier = 0;
                        y_Direction_Modifier = -1;
                    }
                }
                else follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
            }
            else follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
            break;

            case OPTION_RIGHT:
            if(!just_Turned)
            {
                Update_movement_feed("Splitsing rechts");
                x_Pos += (1 * x_Direction_Modifier);
                y_Pos += (1 * y_Direction_Modifier);
                if(!y_Direction_Modifier && x_Pos != 0 && (x_Pos % 2 == 0 && (side_Scanned == false || (side_Scanned == true && x_Pos < 0))))
                {
                    driving_State = TURNING_RIGHT;
                    if(x_Direction_Modifier == 1)
                    {
                        x_Direction_Modifier = 0;
                        y_Direction_Modifier = -1;
                    }
                    else
                    {
                        x_Direction_Modifier = 0;
                        y_Direction_Modifier = 1;
                    }
                }
                else follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
            }
            else follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
            break;

            case SPLIT:
            Update_movement_feed("Splitsing");
            x_Pos += (1 * x_Direction_Modifier);
            y_Pos += (1 * y_Direction_Modifier);
            if(y_Direction_Modifier == 1)
            {
                y_Max = y_Pos;
                if(side_Scanned == true)
                {
                    x_Direction_Modifier = -1;
                    y_Direction_Modifier = 0;
                    driving_State = TURNING_LEFT;
                }
                else 
                {
                    x_Direction_Modifier = 1;
                    y_Direction_Modifier = 0;
                    driving_State = TURNING_RIGHT;
                }
            }
            else
            {
                y_Min = y_Pos;
                if(side_Scanned == true)
                {
                    x_Direction_Modifier = -1;
                    y_Direction_Modifier = 0;
                    driving_State = TURNING_RIGHT;
                }
                else 
                {
                    x_Direction_Modifier = 1;
                    y_Direction_Modifier = 0;
                    driving_State = TURNING_LEFT;
                }
            }
            break;

            case LEFT_TURN:
            Update_movement_feed("Bocht links");
            last_Junction = LEFT_TURN;
            x_Pos += (1 * x_Direction_Modifier);
            y_Pos += (1 * y_Direction_Modifier);
            if(x_Direction_Modifier == 0)
            {
                if(y_Direction_Modifier == 1)
                {
                  y_Max = y_Pos;
                  x_Direction_Modifier = -1;
                  y_Direction_Modifier = 0;
                }
                else
                {
                  y_Min = y_Pos;
                  x_Direction_Modifier = 1;
                  y_Direction_Modifier = 0;
                }
                if(side_Scanned == true)
                {
                  reset_Lego();
                  program_State_c = GO_HOME;
                  Update_movement_feed("Going home");
                } 
                else side_Scanned = true;
                driving_State = TURNING_LEFT;
            }
            else
            {
                if(x_Direction_Modifier == 1) x_Max = x_Pos;
                else x_Min = x_Pos;
                if(x_Pos % 2 == 0)
                {
                    if(x_Direction_Modifier == 1)
                    {
                        x_Direction_Modifier = 0;
                        y_Direction_Modifier = 1;
                    }
                    else
                    {
                        x_Direction_Modifier = 0;
                        y_Direction_Modifier = -1;
                    }
                    driving_State = TURNING_LEFT;
                }
                else
                {
                    if(side_Scanned == true) 
                    {
                        reset_Lego();
                        program_State_c = GO_HOME;
                        x_Min = x_Pos;
                        Update_movement_feed("Going home");
                    }
                    else
                    {
                        x_Direction_Modifier = -1;
                        driving_State = TURNING_180;
                        side_Scanned = true;
                    }
                }
            }
            break;

            case RIGHT_TURN:
            Update_movement_feed("Bocht rechts");
            last_Junction = RIGHT_TURN;
            x_Pos += (1 * x_Direction_Modifier);
            y_Pos += (1 * y_Direction_Modifier);
            if(!x_Direction_Modifier)
            {
                if(y_Direction_Modifier == 1)
                {
                    y_Max = y_Pos;
                    x_Direction_Modifier = 1;
                    y_Direction_Modifier = 0;
                }
                else
                {
                    y_Min = y_Pos;
                    x_Direction_Modifier = -1;
                    y_Direction_Modifier = 0;
                }
                if(side_Scanned == true)
                {
                    reset_Lego();
                    program_State_c = GO_HOME;
                    Update_movement_feed("Going home");
                } 
                else if(x_Pos)side_Scanned = true;
                driving_State = TURNING_RIGHT;
            }
            else
            {
                if(x_Direction_Modifier == 1) x_Max = x_Pos;
                else x_Min = x_Pos;
                if(x_Pos % 2 == 0)
                {
                    if(x_Direction_Modifier == 1)
                    {
                        x_Direction_Modifier = 0;
                        y_Direction_Modifier = -1;
                    }
                    else
                    {
                        x_Direction_Modifier = 0;
                        y_Direction_Modifier = 1;
                    }
                    driving_State = TURNING_RIGHT;
                }
                else
                {
                    if(side_Scanned == true) 
                    {
                        reset_Lego();
                        program_State_c = GO_HOME;
                        x_Min = x_Pos;
                        Update_movement_feed("Going home");
                    }
                    else
                    {
                        x_Max = x_Pos;
                        side_Scanned = true;
                        x_Direction_Modifier = -1;
                        driving_State = TURNING_180_RIGHT;
                    }
                }
            }
            break;

            case NO_LINE:
            if(just_Turned > 0)
            {
                drive_Straight(motor_Left_c, motor_Right_c, driving_Speed_c);
            }
            else
            {
                Update_movement_feed("Geen lijn");
                end_Program();
            }
            break;
        }
        break;

        case TURNING_LEFT:
        turning_Left();
        break;

        case TURNING_RIGHT:
        turning_Right();
        break;

        case TURNING_180:
        turning_180();
        break;

        case TURNING_180_RIGHT:
        turning_180_Right();
        break;
    }
    return program_State_c;
}

int algorithm::go_Home()
{
    if(just_Turned > 0) just_Turned++;
    if(just_Turned > wait_Samples_c*4) just_Turned = 0;
    
    switch(driving_State)
    {
        case STRAIGHT:
        if(!x_Pos && !y_Pos) program_State_c = TURN_0;
        else if(x_Direction_Modifier == 1)
        {
            if(x_Pos < 0) program_State_c = GO_X0;
            else if(x_Pos == 0)
            {
                y_Direction_Modifier = 1;
                driving_State = TURNING_LEFT;
                program_State_c = GO_Y0;
            }
            else
            {
                x_Direction_Modifier = -1;
                if(last_Junction == RIGHT_TURN) driving_State = TURNING_180_RIGHT;
                else driving_State = TURNING_180;
            } 
        }
        else if(x_Direction_Modifier == -1)
        {
            if(x_Pos > 0) program_State_c = GO_X0;
            else if(x_Pos == 0)
            {
                y_Direction_Modifier = 1;
                driving_State = TURNING_RIGHT;
                program_State_c = GO_Y0;
            }
            else
            {
                x_Direction_Modifier = 1;
                if(last_Junction == RIGHT_TURN) driving_State = TURNING_180_RIGHT;
                else driving_State = TURNING_180;
            } 
        }
        else if(y_Direction_Modifier == 1)
        {
            if(y_Pos < 0) program_State_c = GO_Y0;
            else if(y_Pos == 0)
            {
                x_Direction_Modifier = 1;
                driving_State = TURNING_RIGHT;
                program_State_c = GO_X0;
            }
            else
            {
                y_Direction_Modifier = -1;
                if(last_Junction == RIGHT_TURN) driving_State = TURNING_180_RIGHT;
                else driving_State = TURNING_180;
            } 
        }
        else if(y_Direction_Modifier == -1)
        {
            if(y_Pos > 0) program_State_c = GO_Y0;
            else if(y_Pos == 0)
            {
                x_Direction_Modifier = 1;
                driving_State = TURNING_LEFT;
                program_State_c = GO_X0;
            }
            else
            {
                y_Direction_Modifier = 1;
                if(last_Junction == RIGHT_TURN) driving_State = TURNING_180_RIGHT;
                else driving_State = TURNING_180;
            }
        }
        break;

        case TURNING_LEFT:
        turning_Left();
        break;

        case TURNING_RIGHT:
        turning_Right();
        break;

        case TURNING_180:
        turning_180();
        break;

        case TURNING_180_RIGHT:
        turning_180_Right();
        break;
    }
    return program_State_c;
}

int algorithm::go_X0()
{
    if(just_Turned > 0) just_Turned++;
    if(just_Turned > wait_Samples_c*4) just_Turned = 0;

    switch(driving_State)
    {
        case STRAIGHT:
        if(!x_Pos)
        {
            if(!y_Pos) program_State_c = TURN_0;
            else if(y_Pos > 0 && x_Direction_Modifier == 1)
            {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = -1;
                driving_State = TURNING_RIGHT;
            }
            else if(y_Pos > 0 && x_Direction_Modifier == -1)
            {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = -1;
                driving_State = TURNING_LEFT;
            }
            else if(y_Pos < 0 && x_Direction_Modifier == 1)
            {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = 1;
                driving_State = TURNING_LEFT;
            }
            else if(y_Pos < 0 && x_Direction_Modifier == -1)
            {
                x_Direction_Modifier = 0;
                y_Direction_Modifier = 1;
                driving_State = TURNING_RIGHT;
            }
            else program_State_c = GO_Y0;
        }
        else
        {
            road = get_Road_Information(data_Location_c, min_Side_Line_Change_c, min_Mid_Line_Change_c, wait_Samples_c, line_Samples_c);
            switch(road)
            {
                case LINE:
                if(!just_Turned) follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                else follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                break;

                case CROSS:
                Update_movement_feed("Kruising");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                break;

                case OPTION_LEFT:
                if(!just_Turned)
                {
                    Update_movement_feed("Splitsing links");
                    x_Pos += (1 * x_Direction_Modifier);
                    follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                }
                break;

                case OPTION_RIGHT:
                if(!just_Turned)
                {
                    Update_movement_feed("Splitsing rechts");
                    x_Pos += (1 * x_Direction_Modifier);
                    follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                }
                break;

                case SPLIT:
                Update_movement_feed("Splitsing");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                break;

                case LEFT_TURN:
                Update_movement_feed("Bocht links");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                break;

                case RIGHT_TURN:
                Update_movement_feed("Bocht rechts");
                x_Pos += (1 * x_Direction_Modifier);
                follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                break;
            }
        }
        break;

        case TURNING_LEFT:
        turning_Left();
        break;

        case TURNING_RIGHT:
        turning_Right();
        break;
    }
    return program_State_c;
}

int algorithm::go_Y0()
{
    if(just_Turned > 0) just_Turned++;
    if(just_Turned > wait_Samples_c*4) just_Turned = 0;

    switch(driving_State)
    {
        case STRAIGHT:
        if(!y_Pos && reached_Y_Min == true)
        {
            if(!x_Pos) program_State_c = TURN_0;
            else if(x_Pos > 0 && y_Direction_Modifier == 1)
            {
                x_Direction_Modifier = -1;
                y_Direction_Modifier = 0;
                driving_State = TURNING_LEFT;
            }
            else if(x_Pos > 0 && y_Direction_Modifier == -1)
            {
                x_Direction_Modifier = -1;
                y_Direction_Modifier = 0;
                driving_State = TURNING_RIGHT;
            }
            else if(x_Pos < 0 && y_Direction_Modifier == 1)
            {
                x_Direction_Modifier = 1;
                y_Direction_Modifier = 0;
                driving_State = TURNING_RIGHT;
            }
            else if(x_Pos < 0 && y_Direction_Modifier == -1)
            {
                x_Direction_Modifier = 1;
                y_Direction_Modifier = 0;
                driving_State = TURNING_LEFT;
            }
            else program_State_c = GO_X0;
        }
        else
        {
            if(y_Pos == y_Min && reached_Y_Min == false)
            {
                if(y_Direction_Modifier != 1)
                {
                    y_Direction_Modifier = 1;
                    if(x_Pos == x_Min) driving_State = TURNING_180_RIGHT;
                    else driving_State = TURNING_180;
                }
                y_Pos = y_Min;
                reached_Y_Min = true;
                break;
            }
            road = get_Road_Information(data_Location_c, min_Side_Line_Change_c, min_Mid_Line_Change_c, wait_Samples_c, line_Samples_c);
            switch(road)
            {
                case LINE:
                if(!just_Turned) follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                else follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                break;

                case CROSS:
                Update_movement_feed("Kruising");
                y_Pos += (1 * y_Direction_Modifier);
                follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                break;

                case OPTION_LEFT:
                if(!just_Turned)
                {
                    Update_movement_feed("Splitsing links");
                    y_Pos += (1 * y_Direction_Modifier);
                    follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                }
                break;

                case OPTION_RIGHT:
                if(!just_Turned)
                {
                    Update_movement_feed("Splitsing rechts");
                    y_Pos += (1 * y_Direction_Modifier);
                    follow_Line(data_Location_c, goal_c, error_Gain_c, rate_Of_Change_c, motor_Left_c, motor_Right_c, driving_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c);
                }
                break;

                case SPLIT:
                Update_movement_feed("Splitsing");
                y_Pos += (1 * y_Direction_Modifier);
                driving_State = TURNING_180;
                y_Direction_Modifier = 1;
                reached_Y_Min = true;
                break;

                case LEFT_TURN:
                Update_movement_feed("Bocht links");
                y_Pos += (1 * y_Direction_Modifier);
                driving_State = TURNING_180;
                y_Direction_Modifier = 1;
                reached_Y_Min = true;
                break;

                case RIGHT_TURN:
                Update_movement_feed("Bocht rechts");
                y_Pos += (1 * y_Direction_Modifier);
                driving_State = TURNING_180_RIGHT;
                y_Direction_Modifier = 1;
                reached_Y_Min = true;
                break;
            }
        }
        break;

        case TURNING_LEFT:
        turning_Left();
        break;

        case TURNING_RIGHT:
        turning_Right();
        break;

        case TURNING_180:
        turning_180();
        break;

        case TURNING_180_RIGHT:
        turning_180_Right();
        break;
    }
    return program_State_c;
}

int algorithm::turn_0(int expected_Container_Numbers, int number_Retries)
{
    char feed_entry[MOV_FEED_WIDTH];

    if(just_Turned > 0) just_Turned++;
    if(just_Turned > wait_Samples_c*4) just_Turned = 0;

    switch(driving_State)
    {
        case STRAIGHT:
        if(y_Direction_Modifier == 1 && !just_Turned)
        {
            static int number_Already_Driven = 0;
            if(expected_Container_Numbers == get_Number_Of_Containers() || number_Already_Driven == number_Retries || expected_Container_Numbers == 0)
            {
                Update_movement_feed("");
                Update_movement_feed("Einde van programma");
                end_Program();
            }
            else if(expected_Container_Numbers > get_Number_Of_Containers())
            {
                Update_movement_feed("Niet alle containers gevonden,");
                Update_movement_feed("opnieuw scannen...");
                Update_movement_feed("");
                Update_movement_feed("");
                number_Already_Driven++;
                reset_Variabels();
            }
            else if(expected_Container_Numbers < get_Number_Of_Containers())
            {     
                Update_movement_feed("");
                Update_movement_feed("Einde van programma");
                sprintf(feed_entry, "%s%d%s", "Er zijn ", get_Number_Of_Containers() - expected_Container_Numbers, " containers teveel gevonden!");
                Update_movement_feed(feed_entry);
                Update_movement_feed("");
                end_Program();
            }
        }
        if(y_Direction_Modifier == -1 && !just_Turned)
        {
            driving_State = TURNING_180;
            y_Direction_Modifier = 1;
            x_Direction_Modifier = 0;
        }
        if(x_Direction_Modifier == 1 && !just_Turned)
        {
            driving_State = TURNING_LEFT;
            y_Direction_Modifier = 1;
            x_Direction_Modifier = 0;
        }
        if(x_Direction_Modifier == -1 && !just_Turned)
        {
            driving_State = TURNING_RIGHT;
            y_Direction_Modifier = 1;
            x_Direction_Modifier = 0;
        }
        break;

        case TURNING_LEFT:
        turning_Left();
        break;

        case TURNING_RIGHT:
        turning_Right();
        break;

        case TURNING_180:
        turning_180();
        break;
    }
    return program_State_c;
}

void algorithm::stop()
{
    Update_movement_feed("Programma handmatig beëindigd");
    end_Program();
}

void algorithm::setup_Motor(uint8_t motor_Left, uint8_t motor_Right, uint16_t driving_Speed, uint16_t turning_Speed)
{
    motor_Left_c = motor_Left;
    motor_Right_c = motor_Right;
    driving_Speed_c = -driving_Speed;
    turning_Speed_c = -turning_Speed;
}

void algorithm::setup_PID(int* data_Location, int goal, float error_Gain, uint8_t rate_Of_Change)
{
    data_Location_c = data_Location;
    goal_c = goal;
    error_Gain_c = error_Gain;
    rate_Of_Change_c = rate_Of_Change;
}

void algorithm::setup_Samples(const uint16_t line_Samples, uint16_t wait_Samples)
{
    line_Samples_c = line_Samples;
    wait_Samples_c = wait_Samples;
}

void algorithm::setup_Contrast(uint16_t min_Side_Line_Change, uint16_t min_Mid_Line_Change)
{
    min_Side_Line_Change_c = min_Side_Line_Change;
    min_Mid_Line_Change_c = min_Mid_Line_Change;
}

void algorithm::turning_Left()
{
    what_Doing = turn_Left(data_Location_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c, wait_Samples_c, line_Samples_c);
    if(what_Doing == TURNING_LEFT) driving_State = what_Doing;
    else
    {
        just_Turned = 1;
        driving_State = what_Doing;
    }
}

void algorithm::turning_Right()
{
    what_Doing = turn_Right(data_Location_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c, wait_Samples_c, line_Samples_c);
    if(what_Doing == TURNING_RIGHT) driving_State = what_Doing;
    else
    {
        just_Turned = 1;
        driving_State = what_Doing;
    }
}

void algorithm::turning_180()
{
    what_Doing = turn_180(data_Location_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c, wait_Samples_c, line_Samples_c);
    if(what_Doing == TURNING_180) driving_State = what_Doing;
    else
    {
        just_Turned = 1;
        driving_State = what_Doing;
    }
}

void algorithm::turning_180_Right()
{
    what_Doing = turn_180_Right(data_Location_c, motor_Left_c, motor_Right_c, turning_Speed_c, min_Side_Line_Change_c, min_Mid_Line_Change_c, wait_Samples_c, line_Samples_c);
    if(what_Doing == TURNING_180_RIGHT) driving_State = what_Doing;
    else
    {
        just_Turned = 1;
        driving_State = what_Doing;
    }
}

void algorithm::end_Program()
{
    print_Found_Containers(x_Min, y_Min, x_Max, y_Max);
    reset_Lego();
    exit(-2);
}