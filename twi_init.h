/*******************************************************
TWI initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

#ifndef _TWI_INIT_INCLUDED_
#define _TWI_INIT_INCLUDED_

// TWI bus interface functions
#include <twi.h>

// General TWI0 initialization
void twi0_init(void);
// Structure that holds information used by the TWI0 Master
// for performing a TWI bus transaction
extern TWI_MASTER_INFO_t twi0_master;

#endif
