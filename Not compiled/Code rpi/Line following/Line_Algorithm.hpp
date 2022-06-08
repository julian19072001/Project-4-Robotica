#ifndef LINE_ALGORITHM_HPP
#define LINE_ALGORITHM_HPP
    #include <unistd.h>
    #include <stdbool.h>
    #include <Line_Follower.hpp>
    #include <container_Detection.hpp>

    #define SEARCH_LINE 0
    #define DRIVE_OVER_GRID 1
    #define GO_HOME 2
    #define GO_X0 3
    #define GO_Y0 4
    #define TURN_0 5

    class algorithm
    {
        public:
        algorithm();

        int search_Line();
        int drive_Over_Grid();
        int go_Home();
        int go_X0();
        int go_Y0();
        int turn_0(int expected_Container_Numbers, int number_Retries);
        void stop();

        void setup_Motor(uint8_t motor_Left, uint8_t motor_Right, uint16_t driving_Speed, uint16_t turning_Speed);
        void setup_PID(int* data_Location, int goal, float error_Gain, uint8_t rate_Of_Change);
        void setup_Samples(uint16_t line_Samples, uint16_t wait_Samples);
        void setup_Contrast(uint16_t min_Side_Line_Change, uint16_t min_Mid_Line_Change);


        int y_Direction_Modifier;
        int y_Pos;
        int y_Max;
        int y_Min;

        int x_Direction_Modifier;
        int x_Pos;
        int x_Max;
        int x_Min;

        int just_Turned;
        int driving_State;

        uint16_t wait_Samples_c;

        bool reached_Y_Min;

        private:
    	void reset_Variabels();
        void turning_Left();
        void turning_Right();
        void turning_180();
        void turning_180_Right();
        void end_Program();

        int last_Junction;
        int road;
        int what_Doing;

        bool side_Scanned;

        int program_State_c;
        int goal_c;
        int* data_Location_c;
        uint8_t rate_Of_Change_c;
        uint8_t motor_Left_c;
        uint8_t motor_Right_c;
        uint16_t driving_Speed_c;
        uint16_t turning_Speed_c;
        uint16_t line_Samples_c;
        uint16_t min_Side_Line_Change_c;
        uint16_t min_Mid_Line_Change_c;
        float error_Gain_c;
    };
    #endif