/*********************************************************************************************************************************************
* Main.c:
* 
* First written on 21/01/2017
* Last modified on 04/04/2017
*
* Author: Kieran Foley & Jake Patch
*
* Program Name: RobotRoom.c
* 
* Program Description: 
* This program will execute instructions for a lab robot to escape a room.
* The robot will move forward slowly until either / both front bumpers are activated. It will then change directed and rotate
* for a random period of time until it finds its way out of the room.
*
********************************************************************************************************************************************/

/****************************************************************************************
/*  Defines & includes section  */
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "alt_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "unistd.h"
#include <time.h>

/* Motor Direction Defines */
#define FORWARD 0xF
#define BACKWARDS 0x03
#define TURN_RIGHT 0x7
#define TURN_LEFT 0xE
#define STOP 0x0

/* Variable for the inputs of both bumper sensors. */
alt_u32 F_bumper_inputs;

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
* This function will allow the robot to move foward slowly constantly checking the front
* bumpers. Once the inputs of the bumpers change, depending on which / both bumper, the robot 
* will stop, reverse slightly and then turn left / right a random amount ultimately allowing
* the robot to find its way out of a room.
*
*****************************************************************************************/
int main() {

    /* Variable for read all the inputs of the robot. */
    IOWR_ALTERA_AVALON_PIO_DIRECTION(EXPANSION_JP1_BASE, 0xF000000F);

    /* Generates random numbers using time as the seed. */
    srand(time(NULL));
    
    while(1) {
        
        /* Retrieves the inputs from the robot into a variable. */
        F_bumper_inputs = IORD_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE);
        /* Mask The front bumper Sensors. */
        F_bumper_inputs = F_bumper_inputs & (0x8000 | 0x800);
        
               
        /* Generates a random integer between the two values past in. */
        int RandRange(int Min, int Max) {
            
            int diff = Max-Min;
            return (int) (((double)(diff+1)/RAND_MAX) * rand() + Min);
        }
    
        /* Calls the random function with a minimum and maximum random number */             
        int r = RandRange(100000, 2000000);
    
        /* This switch statement checks the inputs of the bumper sensors and take actions accordingly. */
        switch(F_bumper_inputs) {
            /* Front left bumper has been activated. Stop, reverse slightly and turn left a random amount. */
            case 0x8000:
                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, STOP);
                usleep(1000000);

                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, BACKWARDS);
                usleep(50000);

                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, TURN_LEFT);
                usleep(r);
            break;
                
            /* Front right bumper has been activated. Stop, reverse slightly and turn right a random amount. */
            case 0x800:
                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, STOP);
                usleep(1000000);

                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, BACKWARDS);
                usleep(50000);

                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, TURN_RIGHT);
                usleep(r);
            break;
                
            /* No bumpers have been activated. Stop and then proceed to move forwards. */
            case 0x8800:
                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, STOP);
                usleep(110000);

                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, FORWARD);
                usleep(25000);
            break;
                
            /* Both bumpers have been activated. Stop, reverse slightly and turn right a random amount. */
            case 0x0:
                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, STOP);
                usleep(1000000);

                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, BACKWARDS);
                usleep(50000);

                IOWR_ALTERA_AVALON_PIO_DATA(EXPANSION_JP1_BASE, TURN_RIGHT);
                usleep(r);
            break;
       }
    }
    return 0;
}