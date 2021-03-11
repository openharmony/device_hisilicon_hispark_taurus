#ifndef __COMMON_H_
#define __COMMON_H_
#include "types.h"
/*-----------------------------------------------------------------
 * set cpu mode interface
------------------------------------------------------------------*/ 
void set_mod_normal();
void set_mod_slow();

/*-----------------------------------------------------------------
 * serial interface
------------------------------------------------------------------*/ 
int serial_init ();
int serial_deinit();
void serial_putc (const char c);
void serial_putchar (const char c);
void serial_puts (const char *s);
void serial_flush();
int serial_getc (void);
int serial_tstc (void);

/*-----------------------------------------------------------------
 * mmc interface
------------------------------------------------------------------*/ 
int mmc_init();
void mmc_deinit();
size_t mmc_read(void *ptr, size_t size);


/*-----------------------------------------------------------------
 * timer interface
------------------------------------------------------------------*/ 
int timer_init();
void timer_deinit();
void timer_start();
unsigned long timer_get_val();
unsigned long timer_get_divider();

/*-----------------------------------------------------------------
 * I cache interface
------------------------------------------------------------------*/ 
void open_I_cache();
void close_I_cache();

/*-----------------------------------------------------------------
 * mem operation interface
------------------------------------------------------------------*/ 
void*  memset(void * s,int c,size_t count);
void*  memcpy(void * dest,const void *src,size_t count);

#endif /*__COMMON_H_*/
