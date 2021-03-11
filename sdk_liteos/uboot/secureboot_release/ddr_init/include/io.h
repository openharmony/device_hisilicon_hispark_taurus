#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#include <types.h>
#include <barriers.h>

#define __arch_getl(a)			(*(volatile unsigned int *)(a))
#define __arch_putl(v,a)        (*(volatile unsigned int *)(a) = (v))

#define mb()        dsb()
#define __iormb()   dmb()
#define __iowmb()   dmb()

#define writel(v,c) ({ u32 __v = v; __iowmb(); __arch_putl(__v,c); __v; })
#define readl(c)    ({ u32 __v = __arch_getl(c); __iormb(); __v; })

#endif  /* __ASM_ARM_IO_H */
