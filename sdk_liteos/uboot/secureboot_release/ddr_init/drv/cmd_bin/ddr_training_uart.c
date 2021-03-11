// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
 *
 * Description:Simple ddr_training driver for the PrimeCell PL010/PL011 UARTs
 */

#include "ddr_training_custom.h"

#define UART_PL01x_DR				   0x00	 /*  Data read or written from the interface. */
#define UART_PL01x_FR				   0x18	 /*  Flag register (Read only). */
#define UART_PL01x_FR_TXFF			  0x20

#define IO_WRITE(addr, val) (*(volatile unsigned int *)(addr) = (val))
#define IO_READ(addr) (*(volatile unsigned int *)(addr))

void uart_early_putc(const char c)
{
	/* Wait until there is space in the FIFO */
	while (IO_READ (DDR_REG_BASE_UART0 + UART_PL01x_FR) & UART_PL01x_FR_TXFF);

	/* Send the character */
	IO_WRITE (DDR_REG_BASE_UART0 + UART_PL01x_DR, c);
}

void uart_early_puts(const char *s)
{
	if (s == NULL)
		return;
	while (*s)
		uart_early_putc (*s++);
}

void uart_early_put_hex(const unsigned int hex)
{
	int i;
	char c;

	for (i = 28; i >= 0; i -= 4) {
		c = (hex >> (unsigned int)i) & 0x0F;
		if (c < 10)
			c += '0';
		else
			c += 'A' - 10;
		uart_early_putc(c);
	}
}
