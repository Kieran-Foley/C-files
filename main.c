/*********************************************************************************************************************************************
* 
* Main.c:
* 
* First written on 11/11/2016 by Kieran Foley.
* Last modified on 23/11/2016 by Kieran Foley.
*
* Author: Kieran Foley
*
* Program Name: Duckshoot
* 
* Program Description:
* This program will execute a game in which you start with every other LED lit on the altera board (0101010101), and depending on the 
* direction (altered by switch 1) will shift the LED's by 1. LED's are then reset back to either the first or last LED once they fall
* off the board creating a continous rotating effect. The aim of this game is to 'shoot' every lit LED using button 2, which will reverse
* the current state of the LED (Shooting will turn off lit LED's and turn on LED's that are off).
* The games difficulty can be modified using the 4 combinations of both switches 6 and 7, this increases / decreases the speed of the LED's.
* Once all LED's are off the game will terminate, you have won.
* If all LED's are lit the game will terminate, you have lost and the ducks have one.
* 
*********************************************************************************************************************************************/

/****************************************************************************************
*  Defines section  */
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "alt_types.h"
#include <stdio.h>
#include <unistd.h>

/****************************************************************************************/

/*****************************************************************************************
*
* Function name : move_leds()
*    returns : led
*    arg1 : dir
*    arg2 : led
* Created by : Kieran Foley
* Date created : 19/11/2016
* Description : This function shifts the LED pattern by 1 either to the left
*               or to the right (depending on the direction(dir)). It also
*               checks to see if an LED has fallen off the 
*               board (for both directions). If so, the LED is reset back 
*               to either 0x01 or 0x200 (first and last LED's). 
*               This creates the visual of a rotating bit pattern through 10 LED's.
*
*****************************************************************************************/
int move_leds(int dir, int led) 
{
    /* Executes if switch 1 is on. */
    if (dir)
    {   
        /* Checks to see if the led has fallen off the board. */
        if (led & 0x200)
        {
            led = led << 1; 
            led = led | 1;
        }
        else 
        {
            /*shifts the LED by 1. */
            led = led << 1;  
        }
    }
    /* Executes if switch 1 is off. */
    else
    {
        /* Checks to see if the led has fallen off the board. */
        if (led & 0x1)
        {
            led = led >> 1; 
            led = led | 0x200;  
        }
        else 
        {
            /* Shifts the LED by 1. */
            led = led >> 1;  
        }
    }  
    return led;
}
/*****************************************************************************************
*
* Function name : seg7()
*    returns : none
*    arg1    : value
* Created by : Kieran Foley
* Date created : 19/11/2016
* Description :  This function displays the amount of 'shots; in denary format
*                on the 7 segment display by passing in the count variable 
*                that is incremented with every button press.
*
*****************************************************************************************/
void Seg7(int value)
{
    /* Array of hex numbers that display 0-9 on the 7 Segment. */
    int values[] =  { 0xC0 ,0xF9, 0xA4, 0xB0, 0x99, 0X92, 0X83, 0XF8, 0x80, 0x98 };
    
    /* Finds the thousands, hundreds, tens and single integers. */
    int dig1 = value / 1000;
    int dig2 = (value % 1000) / 100;
    int dig3 = (value % 100) / 10;
    int dig4 = (value % 10);

    /* Assign all the dig variables to x to display a 4 digit number. */
    int x = values[dig1];
    x <<= 8;
    x |= values[dig2];
    x <<= 8;
    x |= values[dig3];
    x <<= 8;
    x |= values[dig4];

    /* Assign the x variable to the 7 segment board to display the amount of button presses. */
    IOWR_ALTERA_AVALON_PIO_DATA(SEG7_BASE, x);         
}
/*****************************************************************************************
*
* Function name : Switchfunc
*    returns : none
*    arg1 : dir
*    arg2 : speed
* Created by : Kieran Foley
* Date created : 19/11/2016
* Description :  This function checks the state of all 3 switches used (SW0, SW6, SW7)
*                Depending on the state, these switches will change the direction and
*                speed of the LED's (SW0 for direction and SW6 / SW7 for 4 unique
*                speed settings).
*
*****************************************************************************************/
void Switchfunc(int *dir, int *speed)
{
    /* Connects a new variable to the switches on the Altera board. */
    alt_u16 switches = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
    
    /* Different speed values for all possible switch combinations.  */
    int values[8][3] = { {0x01, 1, 4000}, {0x41, 1, 2500}, {0x81, 1, 500}, {0xC1, 1, 1500},
                        {0x00, 0, 4000}, {0x40, 0, 2500}, {0x80, 0, 500}, {0xC0, 0, 1500} };
    
    /* Loops through all elements of the values array until it finds the right combination of switches it is equal to. */                   
    int j = 0;
    for(j = 0; j < 8; j++)
    {
        if(switches == values[j][0])
        {
        /* If the value of the combination of switches is equal to the first value in values, set the specified direction and speed. */
         *dir = values[j][1];
         *speed = values[j][2];   
        }
    }
}
/*****************************************************************************************
*
* Function name : main()
*    returns : 0
* Created by : Kieran Foley
* Date created : 11/11/2016
* Description : main() is the main function of this program. It calls upon all the 
*               other functions (including the button loop which is embedded in 
*               main) in a certain order to make the game work.
*               It will keep executing the loop until all LED's are either off or on. 
*
*****************************************************************************************/
int main()
{
    /* Initialises the speed that will be used to decide how long the sleep should be in the buttons function. */
    int speed;

    /* Boolean variable to decide if the program should keep running after loops (stops the game if it has ended). */
    int run;
    /* Assigns an alternating bit pattern to LED (0101010101). */
    alt_u16 led = 0x2AA;

    /* Decides which way the LED's will be shifted(<< or >>). */
    int dir = 0;

    /* Counter for the amount of 'shots' (button presses) which is passed to the Seg7 function as 'value'. */
    int count = 0;

    /* Sets the variable for the buttons on the altera board. Will be used to see if the user has pressed down a button. */
    alt_u16 buttons=0x00;

    /* Declares the state of a button so the button cannot execute multiple times on one press. */
    alt_u8 state=1;

    /* Variable for the timer to stay in the buttons function. */
    int i;

    /* Hex value for 'FAIL' on 7 segment board. */
    int fail = 0x8E88CFC7;

    /* Hex value for 'PASS' on 7 segment board. */
    int pass = 0x8C889292;
    
    while (run)
    {
        /* Calls the Switchfunc function to decide the direction and speed of the LED's. */
        Switchfunc(&dir, &speed);
        
        /* Calls the move_leds function. */
        led = move_leds(dir, led);

        /* Connects the leds on the Altera board to the value of 'led'. */
        IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, led);
 
        /* Button function to invert the last LED's current state. The programs timer is located in this function so the program is always looking for a button press. */
        for(i = 0; i< 30000; i+= 1000)
        {
            /* Connects the buttons variable to the ALtera board buttons. */
            buttons = IORD_ALTERA_AVALON_PIO_DATA(BUTTONS_BASE);

            /* The inverse of the first (and only) 3 buttons on the Altera board. */
            buttons = (~buttons & 0x07);
            
            /* Executes every time button two pressed down and has the correct state. This 'shoots' the last LED and increases the count variable. */
            if ((buttons & 0x04) && state)
            {
                (count)++;
                led = led ^ 0x200;
                state = 0;
            }
            
            /* User has let go of the button. */
            if (!(buttons & 0x04) && !state)
                state = 1;
           
            /* Slows down the speed of the LED's. */
            usleep(speed);
        }
        
        /* Calling the function for the 7 Segment Display passing in the count variable which is incremented by every button press. */
        Seg7(count);
        
        /* If all LED's are on, the game terminates and 'FAIL' appears on the 7 Segment board. */
        if(led >= 0xFFFF)
        {
            IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, led);
            IOWR_ALTERA_AVALON_PIO_DATA(SEG7_BASE, fail);
            run = 0;
        }
        /* If all LED's are off, the game terminates and 'PASS' appears on the 7 Segment board. */
        else if(!led)
        {
            IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, led);
            IOWR_ALTERA_AVALON_PIO_DATA(SEG7_BASE, pass);
            run = 0;
        }
    }
    return 0;
}