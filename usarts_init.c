/*******************************************************
USARTs initialization created by the
CodeWizardAVR V4.07 Automatic Program Generator
© Copyright 1998-2026 Pavel Haiduc, HP InfoTech S.R.L.
http://www.hpinfotech.ro

Project : DomesticControler
*******************************************************/

// I/O Registers definitions
#include <avr64dd32.h>

// USARTs initialization functions
#include "usarts_init.h"

// USART0 initialization
void usart0_init(void)
{
// Note: The correct PORTA direction for the USART signals
// is configured in the ports_init function.

// The USART0 signals are not remapped: 
// RxD: PORTA.1, TxD: PORTA.0
PORTMUX.USARTROUTEA&= ~PORTMUX_USART0_gm;

// Transmitter is enabled
// Set TxD=1
PORTA.OUTSET=0x01;

// Communication mode: Asynchronous USART
// USART0 Mode: Asynchronous USART
// Data bits: 8
// Stop bits: 1
// Parity: Disabled
USART0.CTRLC=USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc;

// Receive complete interrupt: On
// RS485 Mode: RS485 Disabled
USART0.CTRLA=(1<<USART_RXCIE_bp) | USART_RS485_DISABLE_gc;

// Required Baud rate: 19200
// Real Baud Rate: 19199,0 (x2 Mode), Error: 0,0 %
USART0.BAUD=0x1A0B;

// Receiver: On
// Transmitter: On
// TxD open drain: Off
// Double transmission speed mode: On
// Multi-processor communication mode: Off
USART0.CTRLB=(1<<USART_RXEN_bp) | (1<<USART_TXEN_bp) | (0<<USART_ODME_bp) | USART_RXMODE_CLK2X_gc | (0<<USART_MPCM_bp);
}

// Note: RX_BUFFER_SIZE_USART0 is #define-d in 'usarts_init.h' with the value 32
char rx_buffer_usart0[RX_BUFFER_SIZE_USART0];

#if RX_BUFFER_SIZE_USART0 <= 256
volatile unsigned char rx_wr_index_usart0=0,rx_rd_index_usart0=0;
#else
volatile unsigned int rx_wr_index_usart0=0,rx_rd_index_usart0=0;
#endif

#if RX_BUFFER_SIZE_USART0 < 256
volatile unsigned char rx_counter_usart0=0;
#else
volatile unsigned int rx_counter_usart0=0;
#endif

// This flag is set on USART0 Receiver buffer overflow
bit rx_buffer_overflow_usart0=0;

// USART0 Receiver interrupt service routine
interrupt [USART0_RXC_vect] void usart0_rx_isr(void)
{
unsigned char status;
char data;

status=USART0.RXDATAH;
data=USART0.RXDATAL;
if ((status & (USART_FERR_bm | USART_PERR_bm | USART_BUFOVF_bm)) == 0)
   {
   rx_buffer_usart0[rx_wr_index_usart0++]=data;
#if RX_BUFFER_SIZE_USART0 == 256
   // special case for receiver buffer size=256
   if (++rx_counter_usart0 == 0) rx_buffer_overflow_usart0=1;
#else
   if (rx_wr_index_usart0 == RX_BUFFER_SIZE_USART0) rx_wr_index_usart0=0;
   if (++rx_counter_usart0 == RX_BUFFER_SIZE_USART0)
      {
      rx_counter_usart0=0;
      rx_buffer_overflow_usart0=1;
      }
#endif
   }
}

// Receive a character from USART0
// USART0 is used as the default input device by the 'getchar' function
// #define _ALTERNATE_GETCHAR_ is inserted for this purpose
// in the main program source file before #include <stdio.h>
#pragma used+
char getchar(void)
{
char data;

while (rx_counter_usart0==0);
data=rx_buffer_usart0[rx_rd_index_usart0++];
#if RX_BUFFER_SIZE_USART0 != 256
if (rx_rd_index_usart0 == RX_BUFFER_SIZE_USART0) rx_rd_index_usart0=0;
#endif
#asm("cli")
--rx_counter_usart0;
#asm("sei")
return data;
}
#pragma used-

// Write a character to the USART0 Transmitter
// USART0 is used as the default output device by the 'putchar' function
// #define _ALTERNATE_PUTCHAR_ is inserted for this purpose
// in the main program source file before #include <stdio.h>
#pragma used+
void putchar(char c)
{
while ((USART0.STATUS & USART_DREIF_bm) == 0);
USART0.TXDATAL=c;
}
#pragma used-

