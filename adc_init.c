/*******************************************************
ADC initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

// ADC initialization functions
#include "adc_init.h"

// ADC0 initialization
void adc0_init(void)
{
// Run in standby: Off
// Resolution: 12 Bits
// Operating mode: Single-ended (unsigned result)
// Left adjust result: Off
// Free Running mode: Off
ADC0.CTRLA=(0<<ADC_RUNSTBY_bp) | ADC_RESSEL_12BIT_gc |
    (0<<ADC_CONVMODE_bp) | (0<<ADC_LEFTADJ_bp) | (0<<ADC_FREERUN_bp);

// Sample accumulation: 16 Results
//ADC0.CTRLB=ADC_SAMPNUM_ACC16_gc;

// Sample accumulation: 8 Results
//ADC0.CTRLB=ADC_SAMPNUM_ACC8_gc;

// Sample accumulation: No Accumulation
ADC0.CTRLB=ADC_SAMPNUM_NONE_gc;

// ADC Voltage reference: VDD Pin
// ADC Voltage reference always enabled: Off
VREF.ADC0REF=(0<<VREF_ALWAYSON_bp) | VREF_REFSEL_VDD_gc;

// ADC0 Peripheral clock divisor: 64 (ADC0 clock frequency: 250,000 kHz)
ADC0.CTRLC=ADC_PRESC_DIV64_gc;

// Initialization delay: 16 ADC clock cycles (64,0 us)
// Sampling delay: 5 ADC clock cycles (20,0 us)
ADC0.CTRLD=ADC_INITDLY_DLY16_gc | 5;

// Sample length: 5 ADC clock cycles (20,0 us)
ADC0.SAMPCTRL=3;

// Window comparator mode: Disabled
ADC0.CTRLE=ADC_WINCM_NONE_gc;

// Use the event system to start an AD conversion: Off
ADC0.EVCTRL=0<<ADC_STARTEI_bp;

// Window compare interrupt: Off
// AD conversion finished interrupt: Off
ADC0.INTCTRL=(0<<ADC_WCMP_bp) | (0<<ADC_RESRDY_bp);

// Enable ADC0
ADC0.CTRLA|=(1<<ADC_ENABLE_bp);
}

// Function used to read the AD conversion result for an ADC0 input
// Positive input values: ADC_MUXPOS_AIN1_gc..ADC_MUXPOS_AIN31_gc, 
// ADC_MUXPOS_DAC0_gc, ADC_MUXPOS_DACREF0_gc, 
// ADC_MUXPOS_VDDDIV10_gc, ADC_MUXPOS_VDDIO2DIV10_gc, 
// ADC_MUXPOS_TEMPSENSE_gc, ADC_MUXPOS_BG_TEMPSENSE_gc, 
// ADC_MUXPOS_GND_gc
// Note: The result will be the sum of multiple accumulated conversions
// specified by the sample accumulation setting
unsigned int adc0_read(ADC_MUXPOS_t input)
{
// Wait for an eventual AD conversion to finish
while (ADC0.COMMAND & ADC_STCONV_bm);
// Select the ADC0 positive input
ADC0.MUXPOS=input;
// Start the AD conversion
ADC0.COMMAND=ADC_STCONV_bm;
// Wait for the AD conversion to finish
while ((ADC0.INTFLAGS & ADC_RESRDY_bm)==0);
// Return the AD conversion result
// Note: the RESRDY interrupt flag is cleared by reading both RESL and RESH
return ADC0.RESL+((unsigned int) ADC0.RESH<<8);
}

