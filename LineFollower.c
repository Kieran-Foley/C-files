/*********************************************************************************************************************************************
* Main.c:
* 
* First written on 21/01/2017
* Last modified on 04/04/2017
*
* Author: Kieran Foley & Jake Patch
*
* Program Name: RobotLineFollower.c
* 
* Program Description: 
* This program will execute instructions for a lab robot to follow a line using the sensors underneath itself.
* The robot can be stopped by activating the bumpers infront of it (standing infront of the robot). 
* If taken off the line, the robot will change state into 'finding the line'.
* 
*********************************************************************************************************************************************/

/****************************************************************************************
/* Defines & includes section */
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "alt_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "unistd.h"
/* Eye Switch Values */
#define LEFT_EYE_SWITCH 0x10000
#define RIGHT_EYE_SWITCH 0x20000
 
/* Motor Direction Defines */
#define FORWARD 0xF
#define BACKWARDS 0x03
#define TURN_RIGHT 0x7
#define TURN_LEFT 0xE
#define STOP 0x0
 
/* Line Sensor Defines */
#define NO_SENSORS 0x6000
#define LEFT_SENSOR 0x2000
#define RIGHT_SENSOR 0x4000
#define BOTH_SENSORS 0x0
 
/* Front Bumper Sensor Defines */
#define FRONT_RIGHT_BUMPER 0x800
#define FRONT_LEFT_BUMPER 0x8000
#define FRONT_BUMPERS 0x0
/****************************************************************************************/

/*****************************************************************************************
*
* Function name : main()
*    returns : 0
*    arg1 : N/A
*    arg2 : N/A
* Created by : Kieran Foley & Jake Patch
* Date created : 21/01/2017
* Date last modified : 04/04/17
* Description : 
* This function will check the inputs of the line sensors and follow a line with its left sensors staying on the line
* and the right sensor off the line. Depending on which sensors come off / on the line, the robot will change direction.
* Activating any or both front bumpers of the robot will force the robot to stop all movement.
*
*****************************************************************************************/
int main() {
    /* Variable for the inputs of both line followers. */
    alt_u32 LF_inputs;
    /* Variable for the inputs of both bumper sensors. */
    alt_u32 F_bumper_inputs;
    /* Variable for read all the inputs of the robot. */
    IOWR_ALTERA_AVALON_PIO_DIRECTION(EXPANSION_JP1_BASE, 0xF000000F);
    
    /* timeOut works as a count to put the robot into 'find the line' state. */
    int timeOut = 0;

    while(1) {
        
        /* Retrieves the inputs from the robot into a variable. */
        LF_inputs = IORD_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE);
        /* Mask The Line Follower Sensors. */
        LF_inputs = LF_inputs & (0x4000 | 0x2000);
        
        /* Retrieves the inputs from the robot into a variable. */
        F_bumper_inputs = IORD_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE);
        /* Mask The Front Bumper Sensors */
        F_bumper_inputs = F_bumper_inputs & (0x8000 | 0x800);
               
        /* Something is infront of the robot and therefore it should halt any further movement. */
        if (F_bumper_inputs == FRONT_RIGHT_BUMPER || F_bumper_inputs == FRONT_BUMPERS || F_bumper_inputs == FRONT_LEFT_BUMPER ) {
            IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, STOP);
            usleep(100);
            timeOut = 0;
        /* Both sensors are on the line, the robot will turn right. */
        } else if (LF_inputs == BOTH_SENSORS) {
            IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, TURN_RIGHT);   
            timeOut = 0;
        /* Only the right sensor is on the line, the robot will turn right.  */
        } else if (LF_inputs == RIGHT_SENSOR) {
            IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, TURN_RIGHT);  
            timeOut = 0; 
        /* Both sensors are off the line, the robot will turn left until 'timeOut' has reached 5,000. */
        } else if (LF_inputs == NO_SENSORS) {
            /* 'timeOut' has reached 5,000. Travel forward. */
            if (timeOut >= 5000) {
                    IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, FORWARD);
                    usleep(1000);
            } 
            timeOut++;
            IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, TURN_LEFT);
            
        /* Only the left sensor is on the line, the robot will travel in a straight line. */
        } else if (LF_inputs == LEFT_SENSOR) {
            IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, FORWARD);  
            timeOut = 0;
        }
        
        /* This usleep depicts the speed of the robot. */
        usleep(1000);
        IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, 0x00000000);
        usleep(1000);
    }
    return 0;
}