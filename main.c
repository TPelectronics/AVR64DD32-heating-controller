/*******************************************************
This program was created by the CodeWizardAVR V4.07 
Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
Version : 1.00
Date    : 14.06.2026
Author  : Th Pottier
Company : TP electronics
Comments: 
2026-06-14:
Make init file to test HW


Chip type               : AVR64DD32
Program type            : Application
AVR Core Clock frequency: 16,000000 MHz
Memory model            : Small
Data Stack size         : 2048
*******************************************************/

// I/O Registers definitions
#include <avr64dd32.h>

// USART0 is used as the default input
// device by the 'getchar' function.
#ifndef _ALTERNATE_GETCHAR_
#define _ALTERNATE_GETCHAR_
#endif
// USART0 is used as the default output
// device by the 'putchar' function.
#ifndef _ALTERNATE_PUTCHAR_
#define _ALTERNATE_PUTCHAR_
#endif
// Standard Input/Output functions
#include <stdio.h>

// Delay functions
#include <delay.h>

// Clock System initialization function
#include "clock_init.h"

// I/O Ports initialization function
#include "ports_init.h"

// Timers/Counters initialization functions
#include "timers_init.h"

// RTC initialization function
#include "rtc_init.h"

// USARTs initialization functions
#include "usarts_init.h"

// ADC initialization functions
#include "adc_init.h"

// TWI initialization functions
#include "twi_init.h"

// Declare your global variables here
#include "routine.h"
#include "LCD_meny.h"

void main(void)
{
    // Declare your local variables here
    unsigned char n;

//    unsigned char temp=0;
    unsigned char ActivKey;
    // old value for change
    unsigned int old_year=0;
    unsigned char s = 99; // Initialize to 99 so it triggers a print immediately on boot   
    // Local snapshot variables for formatting
    unsigned char m, h, d, mon;
    unsigned int y;   
    unsigned int CurrentRawTemp; 
    unsigned char IndexClock=0; 
    unsigned int cpu_temp; 
    unsigned char cpu_temp_loop=0;
    unsigned int Vcpu=0; 
    unsigned char MenyValue=0;
    unsigned char LcdIsOff = 0;   // 
     
// Interrupt system initialization
// Optimize for speed
#pragma optsize- 
// Make sure the interrupts are disabled
#asm("cli")
// Round-robin scheduling for level 0 interrupt: Off
// The interrupt vectors will be placed at the start of the Application FLASH section
n=0;
CPU_CCP=CCP_IOREG_gc;
CPUINT.CTRLA=n;
// Restore optimization for size if needed
#pragma optsize_default

// The vectors with lower addresses will have
// higher interrupt level 0 priority (default)
CPUINT.LVL0PRI=0;

// The higher interrupt priority level 1 is not used
CPUINT.LVL1VEC=0;

// System clocks initialization
system_clocks_init();

// Brown-Out Detector and Voltage Level Monitor initialization
// The settings below are applied to the BODCFG fuse
// that will be programmed if the
// Project|Configure|After Build|Action: Program the Chip|Program Fuses
// menu option is enabled in the IDE
// BOD operation in Active or Idle modes: Enabled
// BOD operation in Standby or Power-Down sleep modes: Disabled
// BOD level: 2,45V
// BODCFG=0x24

// The Multi-Voltage I/O is disabled by programming the
// MVSYSCFG bits of the SYSCFG1 fuse to Single-Supply configuration
// SYSCFG1=0x10
// This setting will be applied if the
// Project|Configure|After Build|Action: Program the Chip|Program Fuses
// menu option is enabled in the IDE

// I/O Ports initialization
ports_init();

// Timer/Counter TCA0 initialization
tca0_init();

// Timer/Counter TCB0 initialization
tcb0_init();

// RTC initialization
rtc_pit_init();

// USART0 initialization
usart0_init();

// TWI0 initialization
twi0_init();

// ADC0 initialization
adc0_init();

// Globally enable interrupts
#asm("sei")
// debugg
// use PA.5 for see RTC
 PORTA.DIR |= PIN5_bm;
 
    PWM_alarm=0;
    PWM_sys=0; 
    // update day / month / year for eeprom
    // hours, minut and sec with charge to much eeprom. will see later.
    // Init variable
    LoadFromEEprom ();
    leap_year_cycle = is_leap_year (current_year);
    old_year= current_year;

// init LCD 
    lcd_init ();
//    LoadCustomLCDChars();     // 6 chars into CGRAM, once
  

    
/*   Font 5x16 but not pretty 
    lcd_cmd(0x35); delay_ms(5);    // //FUNCTION SET 001,DL:1,N:0,DH:1,IS2:0,IS1:1= extension
	lcd_cmd(0x80);   
   lcd_char('H');lcd_char('E');lcd_char('L');lcd_char('L');lcd_char('O');
    delay_ms (2000); 
*/
set_backlight_pwm (BackLight);     
set_system_led_pwm ( 100);
set_alarm_led_pwm ( 0);
RELAY1_OFF();
RELAY2_ON(); 
lcd_cmd(0x01); delay_ms(LCD_CDE); 
// init all variable , lcd contrast, lcd backlight ++++
    if ((SavedYear >= 2026) && (SavedYear <= 2099)) 
    {
        current_year = SavedYear;
    } 
    else 
    {
        current_year = Default_Year;
    }
           
    // debugg
    TimeOff=1;
    SetFlagTurnOff = 0;
    TurnOffCountdown = 0;
    // CLOCK RECOVERY VALIDATION
    if (minutes > 59) minutes = 0;
    if (hours > 23)   hours = 0;
while (1)
      {

            // save clock every xx min 
            // 10 to 20min 
            // buffer of 50 value
            if ( NewMinutes >= MAX_MINUT_TO_SAVE )
            {   NewMinutes=0;
                save_time ( hours, minutes);
            }
            
            // if year change update leap_year
            if ( old_year != current_year)
            {   old_year = current_year; // update
                leap_year_cycle = is_leap_year (current_year);
            } 
            
            // print clock on UART
            #asm("cli")
            if (s != seconds)
            {
                // Take a full snapshot of the whole clock immediately while interrupts are paused
                s = seconds;
                m = minutes;
                h = hours;
                d = day_of_month;
                mon = month;
                y = current_year;
                #asm("sei") // Re-enable interrupts immediately after copying    
                
                CurrentRawTemp = adc0_read ( NTC1_PORT ); 
                CurrentTemp1= get_ntc_temperature (CurrentRawTemp);   
                CurrentRawTemp = adc0_read ( NTC2_PORT );
                CurrentTemp2= get_ntc_temperature (CurrentRawTemp);   
                                                                 
                // 3. PRINT THE TIME on uart(Executes exactly once every second)
                printf("\r\nTime: %02d:%02d:%02d  Date: %02d/%02d/%04d\r\n", h, m, s, d, mon, y);
                //refresh_lcd_ints ( CurrentTemp, hours, minutes, seconds); 
//                refresh_lcd_temps(CurrentTemp1,CurrentTemp2); 
//                LoadCustomBlock (); 
//                PrintBlock ( IndexClock,15,2);  
//                if (IndexClock++ > 7)
//                    IndexClock=0;
//                set_alarm_led_pwm ( PWM_alarm);
                cpu_temp_loop++;         
                if ( cpu_temp_loop > 9)
                {     
                    cpu_temp = ReadCPUtemperature();
                    cpu_temp_loop=1; 
                    Vcpu= ReadSystemVCC();
                    cpu_temp_loop=0; 
					CurrentRawTemp = adc0_read ( NTC1_PORT ); 
					CurrentRawTemp = adc0_read ( NTC2_PORT );
                }
                RELAY1_TOGGLE ();
                RELAY2_TOGGLE ();
            }
            else
            {
                #asm("sei") // Re-enable interrupts if the second did not change
            } 
 
            // Test KeyBoard
            ActivKey=ScanKeyBoard ();  
            if (ActivKey != KEY_NONE)
            {   SetFlagTurnOff =0;
                TurnOffCountdown = 0;
                if (LcdIsOff == 1)
                {
                    lcd_cmd(0x0C);
                    set_backlight_pwm(BackLight);
                    LcdIsOff = 0;
                }
            }
            if (TimeOff == 0)                  // NEVER: keep flag/count dead
            {
                SetFlagTurnOff = 0;
                TurnOffCountdown = 0;
            }
            if (SetFlagTurnOff == 1)
            {   if (LcdIsOff == 0)
                {   lcd_cmd(0x08); 
                    set_backlight_pwm(0);            // backlight off   
                    set_system_led_pwm(10);
                    
                    LcdIsOff = 1;
                }
            }
            // make meny choice
            if ( ActivKey == KEY_OK ) 
            {   while ( ActivKey)   // wait release
                {   ActivKey=ScanKeyBoard ();
                }    
                //run_clock_setting_menu();  
                //SetupDateSimple();  
                MenyValue= DisplayMeny (); 

                switch ( MenyValue )
                {
                    case 1:  // Set Dato 
                    //Need EEPROM for month, days, year but char 26 to 255 
                        SetupDateSimple();                               
                    break;
                                                    
                    case 2:  // Set contrast
                        SetupContrast();
                                                          
                    break; 
                                                        
                    case 3:  // Set back light 
                        SetupBacklight();
                                                           
                    break; 
                                                   
                    case 4:  // Set Modus
                        //SetupOverride ();
                        SetupMode(); 
                        if (Mode == MODE_COMBI)
                        {
                            SetupOverride();   // seasonal setup — only for Combi/UFloor
                        }
                              
                    break;
                                                   
                    case 5:  // Set temperature ( with current modus)   
                        SetupDiffSummer ();
                                                        
                    break;
                                                   
                    case 6:  // time for turn off  
                        SetupDiffWinter ();

                    break; 
                    
                    case 7:
                        SetupTimeOff ();

                    break;
                                                   
                    default: // Set Clock  
                    //Need EEPROM for hours ? limit to eeprom live time
                            run_clock_setting_menu(); 
                    break;
                }
                SetFlagTurnOff =0;
                TurnOffCountdown = 0;
                LcdIsOff = 0;  
                while (ScanKeyBoard() != KEY_NONE)
                {
                    // use wdr if issue
                }
                
           }
            
//            while (ScanKeyBoard() != KEY_NONE)   // wait release after ANY submenu
//            {
//                delay_ms(10);
//            }  
            ActivKey= ScanKeyBoard();
            if (ActivKey == KEY_RIGHT)
            {
                MainScreen++;
                if (MainScreen > 2) MainScreen = 0;
                MainDisplay(MainScreen);
            }
            else if (ActivKey == KEY_LEFT)
            {
                if (MainScreen == 0) MainScreen = 2;
                else MainScreen--;
                MainDisplay(MainScreen);
            }

            // ---- repaint once per second ----
            if (seconds != LastSeconds)
            {
                LastSeconds = seconds;
                MainDisplay(MainScreen);
            }

      } // main while
}       // main routine
