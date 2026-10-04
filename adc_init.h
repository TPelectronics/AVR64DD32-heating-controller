/*******************************************************
ADC initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

#ifndef _ADC_INIT_INCLUDED_
#define _ADC_INIT_INCLUDED_

// I/O Registers definitions
#include <avr64dd32.h>

// ADC0 initialization
void adc0_init(void);
// Function used to read the AD conversion result for an ADC0 input
// Positive input values: ADC_MUXPOS_AIN1_gc..ADC_MUXPOS_AIN31_gc, 
// ADC_MUXPOS_DAC0_gc, ADC_MUXPOS_DACREF0_gc, 
// ADC_MUXPOS_VDDDIV10_gc, ADC_MUXPOS_VDDIO2DIV10_gc, 
// ADC_MUXPOS_TEMPSENSE_gc, ADC_MUXPOS_BG_TEMPSENSE_gc, 
// ADC_MUXPOS_GND_gc
// Note: The result will be the sum of multiple accumulated conversions
// specified by the sample accumulation setting
unsigned int adc0_read(ADC_MUXPOS_t input);

#endif
