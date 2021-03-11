include cfg.mk

export CROSS_COMPILE := $(shell pwd)/../../../../../../../prebuilts/gcc-arm-none-eabi-7-2017-q4-major/bin/arm-none-eabi-

export CC := $(CROSS_COMPILE)gcc
export AR := $(CROSS_COMPILE)ar
export RANLIB := $(CROSS_COMPILE)ranlib
export LD := $(CROSS_COMPILE)ld
export OBJCOPY := $(CROSS_COMPILE)objcopy
export OBJDUMP := $(CROSS_COMPILE)objdump

CSRC  = $(wildcard *.c) 
SSRC  = $(wildcard *.S) 

OBJS := $(patsubst %.c,%.o,$(CSRC) )
OBJS += $(patsubst %.S,%.o,$(SSRC) )

export CFLAGS := -fno-builtin -fno-common
CFLAGS += -Wall -D__KERNEL__ -DTEXT_BASE=$(TEXT_BASE) 
CFLAGS += -I$(PWD)/include/

CFLAGS += -march=armv7-a
CFLAGS += -mno-unaligned-access

CFLAGS += -c

CFLAGS += -O2

TARGET = ddr_init

LDSCRIPT := linker.lds
LDFLAGS := -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE)
LDFLAGS += drv/libdrv.a

.SILENT:

default: 
	make -C drv/
	make -C boot/
	sed -e 's/RAM_BASE/$(RAM_BASE)/' \
		$(LDSCRIPT).mk > $(LDSCRIPT)
		
	echo "  LD    $(OBJS) -Map $(TARGET).map -o $(TARGET)"
	$(LD) $(OBJS) $(LDFLAGS) -Map $(TARGET).map -o $(TARGET)
	$(OBJCOPY) -O binary $(TARGET) $(TARGET).bin
	
	mkdir -p ./out
	$(OBJDUMP) -D $(TARGET) > deasm.s
	cp $(TARGET).map ./out
	mv deasm.s ./out

clean:
	echo "  rm    boot/*.o"
	make -C boot/ clean
	echo "  rm    drv/*.o libdrv.a"
	make -C drv/ clean
	echo "  rm    $(TARGET) $(TARGET).bin* ./out"
	rm -rf $(TARGET) $(TARGET).map $(TARGET).bin* ./out

