/*
 * File:   main.c
 * Author: tothpetiszilard
 *
 * Created on 2016. december 23., 15:12
 */

// PIC16F506 Configuration Bit Settings

// CONFIG
#pragma config OSC = IntRC_RB4EN// Oscillator Selection bits (INTRC With RB4 and 1.125 ms DRT)
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled)
#pragma config CP = OFF         // Code Protect (Code protection off)
#pragma config MCLRE = OFF      // Master Clear Enable bit (RB3/MCLR pin functions as RB3, MCLR tied internally to VDD)
#pragma config IOSCFS = ON      // Internal Oscillator Frequency Select bit (8 MHz INTOSC Speed)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

typedef enum CAR_EN
{
    UNLOCKED_OPEN = 0U,
    UNLOCKED_CLOSED = 1U,
    LOCKED_CLOSED = 2U,
    CLOSING_WINDOWS = 3U
}car_state_t;

#define DOOR_SEN    PORTBbits.RB2
#define LOCK_SEN    PORTBbits.RB4
#define AUX_SEN     PORTBbits.RB5


#define DOOR_OUT    PORTCbits.RC0
#define WINDOW1     PORTCbits.RC1
#define WINDOW2     PORTCbits.RC2
#define LED1        PORTCbits.RC3
#define LED2        PORTCbits.RC4
#define LED3        PORTCbits.RC5

#define WINDOWS_MULTIPLIER (2U)
#define WINDOWS_DELAY 65535U

car_state_t car_status;

unsigned int windows_timer;
unsigned char windows_cnt;

void main(void) 
{
    TRISC = 0x00;
    ADCON0 = 0x00;
    TRISB = 0xFF;
    LED1 = 1;
    LED2 = 1;
    LED3 = 1;
    DOOR_OUT = 1;
    LED1 = 0;
    car_status = UNLOCKED_OPEN;
    windows_timer = WINDOWS_DELAY;
    windows_cnt = WINDOWS_MULTIPLIER;
    unsigned char door_u8;
    unsigned char lock_u8;
    while (1)
    {
        door_u8 = DOOR_SEN;
        lock_u8 = LOCK_SEN; 
        
        switch(car_status)
        {
            case UNLOCKED_OPEN:
            {
                if ((0 == door_u8) && (0 == lock_u8)) // Door closed and Vehicle unlocked
                {
                    DOOR_OUT = 0;
                    LED1 = 1;
                    car_status = UNLOCKED_CLOSED;
                }
                break;
            }
            case UNLOCKED_CLOSED:
            {
                if ( 0 != door_u8 ) // Door opened
                {
                    
                    DOOR_OUT = 1;
                    LED1 = 0;
                    car_status = UNLOCKED_OPEN;
                }
                else // Door closed
                {
                    if (0 != lock_u8) // Door closed and vehicle just locked
                    {
                        /* Vehicle locked, start locking process */
                        DOOR_OUT = 1; // Output for BSI
                        LED1 = 1;   // Switching off door led
                        WINDOW1 = 1; // Closing left window
                        WINDOW2 = 1; // Closing right window
                        LED2 = 0; // Turning on left window led
                        LED3 = 0; // Turning on right window
                        windows_timer = WINDOWS_DELAY; // Reloading the soft timer for windows closing function
                        windows_cnt = WINDOWS_MULTIPLIER; // Reloading the multiplier value for the soft timer
                        car_status = CLOSING_WINDOWS;
                    }
                }
                break;
            }
            case CLOSING_WINDOWS: // Vehicle locked and windows are closing right now
            {
                if ((0 != windows_timer) || (0 != windows_cnt)) // If there is more time to wait
                {
                    if (0 != windows_timer) // If timer still running
                    {
                        windows_timer--; // Decrease timer variable
                    }
                    else // If timer is null, but there is still a need to wait (multiplier)
                    {
                        windows_timer = WINDOWS_DELAY; // Reload the timer
                        windows_cnt--; // Decrease multiplier variable
                    }
                }
                else // Closing time is elapsed
                {
                    WINDOW1 = 0; // Stop window closer motor
                    WINDOW2 = 0; // Stop window closer motor
                    LED2 = 1;   // Turn off window led
                    LED3 = 1;
                    car_status = LOCKED_CLOSED; // Windows are closed, switch to locked state
                }
                /* Handling the following: unlocking the car when windows are closing */
                if (( 0 != door_u8 ) || (0 == lock_u8)) // If door is opened OR vehicle unlocked
                {
                    WINDOW1 = 0; // Stop motors
                    WINDOW2 = 0;
                    LED2 = 1;   // Switch off led
                    LED3 = 1;
                    /* Switching door output to unlocked state */
                    if ( 0 == door_u8 )
                    {
                        DOOR_OUT = 0; // Door is still closed
                        LED1 = 1;
                        car_status = UNLOCKED_CLOSED; /* Something happened, switch to unlocked state */
                    }
                    else
                    {
                        DOOR_OUT = 1; // Door is opened
                        LED1 = 0;    // Turn on door led
                        car_status = UNLOCKED_OPEN; /* Something happened, switch to unlocked state */
                    }
                }
                break;
            }
            case LOCKED_CLOSED:
            {
                if (0 == lock_u8) // Vehicle unlocked
                {
                    if ( 0 == door_u8 ) // Is the door closed?
                    {
                        DOOR_OUT = 0; // Door is still closed
                        LED1 = 1;
                        car_status = UNLOCKED_CLOSED; /* Switch to unlocked state */
                    }
                    else
                    {
                        DOOR_OUT = 1; // Door is opened
                        LED1 = 0;    // Turn on door led
                        car_status = UNLOCKED_OPEN; /* Switch to unlocked state */
                    }
                }
                break;
            }
            default:
                break;
        }
        
    }
}
