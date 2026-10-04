/*******************************************************
Timers/Counters initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

// I/O Registers definitions
#include <avr64dd32.h>

// Disable a Timer/Counter type A
void tca_disable(TCA_t *ptca)
{
// Timer/Counter TCA is disabled
ptca->SINGLE.CTRLA=0<<TCA_SINGLE_ENABLE_bp;
// Operate in 16-bit mode
ptca->SINGLE.CTRLD=0<<TCA_SINGLE_SPLITM_bp;
// Issue a reset command
ptca->SINGLE.CTRLECLR=TCA_SINGLE_CMD_RESET_gc;
}

// Timer/Counter TCA0 initialization
void tca0_init(void)
{
// First disable and reset the Timer/Counter TCA0
// Use 16-bit mode
tca_disable(&TCA0);

// Clock divider: 1
// Clock frequency: 16000,000 kHz
// TCA0 runs in standby: Off
TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1_gc+(0<<TCA_SINGLE_RUNSTDBY_bp);

// Operating mode: Single Slope PWM 16-bit OVF=BOTTOM
// Set the waveform outputs configuration:
// WO0: PORTC, Pin 0
// WO1: PORTC, Pin 1
// WO2: PORTC, Pin 2
TCA0.SINGLE.CTRLB=TCA_SINGLE_WGMODE_SINGLESLOPE_gc+
    (1<<TCA_SINGLE_CMP0EN_bp)+
    (1<<TCA_SINGLE_CMP1EN_bp)+
    (1<<TCA_SINGLE_CMP2EN_bp);

// Set the waveform outputs mapping
PORTMUX.TCAROUTEA=(PORTMUX.TCAROUTEA & (~PORTMUX_TCA0_gm)) | PORTMUX_TCA0_PORTC_gc;
// Note: The configuration for the waveform output signals
// is set in the ports_init function from ports_init.c

// Set the Timer Counter register
TCA0.SINGLE.CNT=0x00;

// Set the Timer Period register
TCA0.SINGLE.PER=0x63F;

// Set the Timer Compare 0 register
TCA0.SINGLE.CMP0=0x31F;

// Set the Timer Compare 1 register
TCA0.SINGLE.CMP1=0x31F;

// Set the Timer Compare 2 register
TCA0.SINGLE.CMP2=0x10;

// Set the Event Control register
// Event input A: No action
// Event input B: No action
TCA0.SINGLE.EVCTRL=(0<<TCA_SINGLE_CNTAEI_bp)+(0<<TCA_SINGLE_CNTBEI_bp);

// Set TCA0 interrupts:
// Overflow interrupt: Off
// Compare Channel 0 interrupt: Off
// Compare Channel 1 interrupt: Off
// Compare Channel 2 interrupt: Off
TCA0.SINGLE.INTCTRL=
    (0<<TCA_SINGLE_OVF_bp)+
    (0<<TCA_SINGLE_CMP0_bp)+
    (0<<TCA_SINGLE_CMP1_bp)+
    (0<<TCA_SINGLE_CMP2_bp);

// Initialization finished, enable TCA0
TCA0.SINGLE.CTRLA|=TCA_SINGLE_ENABLE_bm;
}

// Timer/Counter TCB0 initialization
void tcb0_init(void)
{
// Clock divider: 1
// Clock frequency: 16000,000 kHz
// TCB0 runs in standby: Off
TCB0.CTRLA=TCB_CLKSEL_DIV1_gc+(0<<TCB_RUNSTDBY_bp);

// Operating mode: Periodic Interrupt
TCB0.CTRLB=TCB_CNTMODE_INT_gc;

// Set the Timer Counter register
TCB0.CNT=0x00;

// Set the timer period, specified by the CCMP register
TCB0.CCMPL=0x7F;
TCB0.CCMPH=0x3E;

// Set the Event Control register
// The capture event input is disabled
TCB0.EVCTRL=(0<<TCB_CAPTEI_bp);

// TCB0 capture interrupt: On
// The interrupt is triggered when the counter
// reaches the value of the CCMP register
TCB0.INTCTRL=(1<<TCB_CAPT_bp);

// Clear the interrupt flags
TCB0.INTFLAGS=TCB0.INTFLAGS;

// Initialization finished, enable TCB0
TCB0.CTRLA|=TCB_ENABLE_bm;
}

// Timer/Counter TCB0 interrupt service routine
// The interrupt is triggered when the counter
// reaches the value of the CCMP register
interrupt [TCB0_INT_vect] void tcb0_isr(void)
{
// Clear the interrupt flags
TCB0.INTFLAGS=TCB0.INTFLAGS;

// Write your code here

}

