/*******************************************************
System clock initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

// I/O Registers definitions
#include <avr64dd32.h>

// Standard definitions
#include <stddef.h>

void system_clocks_init(void)
{
unsigned char s;

// Optimize for speed
#pragma optsize- 
// Save interrupts enabled/disabled state
s=CPU_SREG;
// Disable interrupts
#asm("cli")
/*
// Ensure that the external 32 kHz oscillator is
// first disabled before modifying its settings
n=0<<CLKCTRL_ENABLE_bp;
CPU_CCP=CCP_IOREG_gc;
CLKCTRL.XOSC32KCTRLA=n;

// Configure the external 32 kHz oscillator
// External source type: Crystal
// Startup time: 1k cycles
// Low-power mode: Off
// Force the external 32 kHz oscillator ON in all sleep modes
n=CLKCTRL_CSUT_1K_gc | (0<<CLKCTRL_SEL_bp) | (0<<CLKCTRL_LPMODE_bp) | CLKCTRL_RUNSTDBY_bm;
CPU_CCP=CCP_IOREG_gc;
CLKCTRL.XOSC32KCTRLA=n;

// Enable the external 32 kHz oscillator
n|=1<<CLKCTRL_ENABLE_bp;
CPU_CCP=CCP_IOREG_gc;
CLKCTRL.XOSC32KCTRLA=n;

// Wait for the external 32 kHz oscillator to be stable before using it
while ((CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm)==0);

// The internal high-frequency oscillator is selected by the OSCCFG.CLKSEL fuse bits=0x00
// Internal high-frequency oscillator frequency: 16 MHz
n=(0<<CLKCTRL_RUNSTDBY_bp) | CLKCTRL_FREQSEL_16M_gc;
CPU_CCP=CCP_IOREG_gc;
CLKCTRL.OSCHFCTRLA=n;

// Set the value of the auto-tune register
CLKCTRL.OSCHFTUNE=0x00;
// Enable auto-tuning the internal high-frequency oscillator
n|=1<<CLKCTRL_AUTOTUNE_bp;
CPU_CCP=CCP_IOREG_gc;
CLKCTRL.OSCHFCTRLA=n;

// Main clock source: 16 MHz Internal HF Oscillator
// Peripheral clock output on CLKOUT (PORTA, Pin 7): On
n=CLKCTRL_CLKSEL_OSCHF_gc | (1<<CLKCTRL_CLKOUT_bp);
CPU_CCP=CCP_IOREG_gc;
CLKCTRL.MCLKCTRLA=n;

// Main clock prescaler division ratio: 1
// CPU and Peripheral clocks: 16000,000 kHz
n=0;
CPU_CCP=CCP_IOREG_gc;
CLKCTRL.MCLKCTRLB=n;
*/
// =========================================================================
    // 1. CONFIGURE SYSTEM MAIN CLOCK (OSCHF) TO 16 MHz
    // =========================================================================
    
    // Set internal High-Frequency Oscillator (OSCHF) frequency to 16 MHz
        CLKCTRL.OSCHFCTRLA = 0x05;        
    // Unlock the main clock source switch register using CodeVision's signature
        CPU_CCP = 0xD8; 
        CLKCTRL.MCLKCTRLA = 0x00; // Select internal OSCHF as main clock
    
    // Unlock and disable the default reset prescaler (divide by 6)
        CPU_CCP = 0xD8;
        CLKCTRL.MCLKCTRLB = 0x00; // 0x00 = Prescaler disabled, run at true 16 MHz



    // =========================================================================
    // 2. CONFIGURE INTERNAL 32.768 kHz OSCILLATOR & RTC CLOCK
    // =========================================================================
    
    // Enable internal 32.768 kHz oscillator (Bit 0 = ENABLE)
    CLKCTRL.OSC32KCTRLA = 0x01;  
    
    // Route the internal 32.768 kHz oscillator directly to the RTC 
    // 0x00 explicitly maps to the internal OSC32K engine
    RTC.CLKSEL = 0x00; 
    

    // Wait until the 32.768 kHz internal oscillator stabilizes (Bit 4 = OSC32KS)
 //       while (!(CLKCTRL.MCLKSTATUS & 0x10));

// Restore interrupts enabled/disabled state
CPU_SREG=s;
// Restore optimization for size if needed
#pragma optsize_default
}

