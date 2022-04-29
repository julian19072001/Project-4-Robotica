#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <legorpi.hpp>

// Create a BrickPi3 instance with the default address of 1
BrickPi3 oLego;

// Method to catch the CTRL+C signal.
void exit_signal_handler(int signo);

// Start of the program
int main(int nArgc, char* aArgv[]) {

  // Register the exit function for Ctrl+C
  signal(SIGINT, exit_signal_handler);

  // Make sure that the BrickPi3 is communicating and the firmware is working as expected.
  oLego.isDetected();

  // Set the type of each connected sensor
  oLego.set_sensor_type(PORT1_S3, SENSOR_TYPE_NXT_LIGHT_ON);
  oLego.set_sensor_type(PORT2_S4, SENSOR_TYPE_NXT_LIGHT_ON);

  // Continue until CTRL+C is pressed
  int nError;
  sensor_light_t      Light3;
  sensor_light_t      Light4;

  while (true) {

    // Start error count
    nError = 0;

    // Read sensor S3
    if (oLego.get_sensor(PORT1_S3, &Light3)) {
      nError++;
    };

    // Read sensor S4
    if (oLego.get_sensor(PORT2_S4, &Light4)) {
      nError++;
    };

    // Four errors mens not all configured
    if (nError == 2) {
      printf("Waiting for sensors to be configured");
    };

    oLego.get_sensor(PORT1_S3, &Light3);
    oLego.get_sensor(PORT2_S4, &Light3);
    
    if(Light3.reflected <= 2000){
        oLego.set_motor_dps(PORT4_MC, 500);
    }else{
        oLego.set_motor_dps(PORT4_MC, -100);
    };

    if(Light4.reflected <= 2000){
        oLego.set_motor_dps(PORT5_MB, 500);
    }else{
        oLego.set_motor_dps(PORT5_MB, -11);
    };

    // Go next line and wait
    printf("\n");
    usleep(20000);
  };
};

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo) {

  if (signo == SIGINT) {
    // Reset everything so there are no run-away motors
    oLego.reset_all();
    printf("\nThe LEGO RPi example has stopped\n\n");
    exit(-2);
  };
};
