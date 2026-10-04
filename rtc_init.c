/*******************************************************
RTC initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

// I/O Registers definitions
#include <avr64dd32.h>
#include <routine.h>


// RTC initialization
void rtc_pit_init(void)
{
    // Wait for any pending asynchronous hardware sync operations to finish
    while (RTC.STATUS & 0x01); // Bit 0 = CTRLAbusy
    
    // Set the Period register (PER). 
    // 32768 Hz / 1 (Prescaler) = 32768 counts for a 1-second interval.
    // CodeVision lets you write a 16-bit value directly to RTC.PER
    RTC.PER = 32768 - 1; 

    // Enable the Overflow Interrupt 
    // 0x01 sets the OVF (Overflow) bit in the INTCTRL register
    RTC.INTCTRL = 0x01; 

    // Enable the RTC peripheral and clear the prescaler divisor
    // 0x01 = Enable RTC, No prescaler (Prescaler = 1)
    RTC.CTRLA = 0x01; 
   
 
}

// RTC interrupt service routine
interrupt [RTC_CNT_vect] void rtc_isr(void)
{
// Clear the RTC interrupt flags
RTC.INTFLAGS=RTC.INTFLAGS;

// Write your code here
PORTA.OUTTGL = PIN5_bm;     // toggle PA.5
    seconds++;
    if (seconds >= 60)
    {   NewMinutes++;
        seconds = 0;
        minutes++;
        SaveCountdown++;
        if (SaveCountdown >= MAX_MINUT_TO_SAVE)    // see value in routine.h
        {   SaveCountdown = 0;
            SetFlagToSave = 1; // Raise the flag, exit the interrupt immediately!
        }
        TurnOffCountdown++;
        if (TurnOffCountdown >= TimeOff)    // see value in routine.h
        {   TurnOffCountdown = 0;
            SetFlagTurnOff = 1; // Raise the flag, exit the interrupt immediately!
        }
        
        if (minutes >= 60)
        {   NewHours++;
            minutes = 0;
            hours++;
            
            if (hours >= 24)
            {   NewDays++;
                hours = 0;
                days++; // Keep incrementing raw total operating days
                
                // =============================================================
                // CALENDAR CONVERSION LOGIC
                // =============================================================
                day_of_month++;
                
                // Step A: Dynamically adjust February limit based on our 4-year cycle tracker
                if (leap_year_cycle == 3)
                {
                    month_lengths[2] = 29; // Leap year February
                }
                else
                {
                    month_lengths[2] = 28; // Normal year February
                }
                
                // Step B: Check if the current day has exceeded the month's maximum allowed days
                if (day_of_month > month_lengths[month])
                {
                    day_of_month = 1; // Reset back to the 1st of the next month
                    month++;          // Move to next month
                    
                    // Step C: Check if the year ended
                    if (month > 12)
                    {
                        month = 1; // Reset back to January
                        current_year++;  // Advance the calendar year automatically!
                    }
                }
        
            }
        }
    }

}

