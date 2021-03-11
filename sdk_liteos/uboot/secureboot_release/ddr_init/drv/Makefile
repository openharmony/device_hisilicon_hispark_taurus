CSRC  = $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(CSRC) )

#.SILENT:

default: libdrv.a

libdrv.a: $(OBJS)
	echo "  AR    libdrv.a"
	$(AR) r libdrv.a $(OBJS)
	echo "  RL    $@"
	$(RANLIB) $@

.c.o:
	echo "  CC    $<"
	$(CC) $(CFLAGS) $<

clean:
	rm *.o *.a -f

