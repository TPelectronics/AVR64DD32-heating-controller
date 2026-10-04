/*******************************************************
Timers/Counters initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

#ifndef _TIMERS_INIT_INCLUDED_
#define _TIMERS_INIT_INCLUDED_

// I/O Registers definitions
#include <io.h>

// Disable a Timer/Counter type A
void tca_disable(TCA_t *ptca);
// Timer/Counter TCA0 initialization
void tca0_init(void);
// Timer/Counter TCB0 initialization
void tcb0_init(void);

#endif
