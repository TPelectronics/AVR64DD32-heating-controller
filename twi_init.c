/*******************************************************
TWI initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

// I/O Registers definitions
#include <avr64dd32.h>

// TWI initialization functions
#include "twi_init.h"

// TWI0 initialization
// Structure that holds information used by the TWI0 Master
// for performing a TWI bus transaction
TWI_MASTER_INFO_t twi0_master;

void twi0_init(void)
{
// General TWI0 initialization
// SDA Setup time: 4 Clock Cycles
// SDA Hold: Off
// Fast Mode+ : Off
twi_init(&TWI0,TWI_SDASETUP_4CYC_gc,TWI_SDAHOLD_OFF_gc,false);
// The TWI0 signals are not remapped:
// Master: SDA: PORTA.2, SCL: PORTA.3
PORTMUX.TWIROUTEA&= ~PORTMUX_TWI0_gm;

// TWI0 Master initialization
// Peripheral Clock frequency: 16000000 Hz
// SCL Rate: 100000 bps
// SCL Rise time: 25 ns
// Real SCL Rate: 99751 bps, Error: 0,2 %
// Inactive bus timeout: Disabled (I2C)
twi_master_init(&twi0_master,&TWI0,
    TWI_BAUD_REG(16000000,100000,25),TWI_TIMEOUT_DISABLED_gc);

// TWI0 Slave is disabled
TWI0.SCTRLA=0;
}

// TWI0 Master interrupt service routine
#pragma optsize- // optimize for speed
interrupt [TWI0_TWIM_vect] void twi0_master_isr(void)
{
twi_master_int_handler(&twi0_master);
}
#pragma optsize_default

