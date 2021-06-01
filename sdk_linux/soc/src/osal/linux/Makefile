# Copyright (C) 2021 HiSilicon (Shanghai) Technologies CO., LIMITED.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

src = $(PWD)
EXTRA_CFLAGS := -I$(src)
EXTRA_CFLAGS += -I$(src)/himedia
EXTRA_CFLAGS += -I$(src)/../include/
EXTRA_CFLAGS += -DHICHIP=$(HICHIP)
ifeq ($(MPP_BUILD), y)
    EXTRA_CFLAGS += $(MPP_CFLAGS)
else
    EXTRA_CFLAGS += -I$(CBB_ROOT)/include
    EXTRA_CFLAGS += -I$(CBB_ROOT)/include/adapt
    EXTRA_CFLAGS += -I$(CBB_ROOT)/based/arch/$(HIARCH)/include/$(HI_SUBARCH)
    EXTRA_CFLAGS += -I$(OHOS_ROOT_PATH)/third_party/bounds_checking_function/include
endif
ifeq ($(CONFIG_DRIVER_SPACE_TYPE),user_space)
EXTRA_CFLAGS :=$(subst base/,based/,$(EXTRA_CFLAGS))
EXTRA_CFLAGS :=$(subst sys/,sysd/,$(EXTRA_CFLAGS))
endif
csrcs = $(wildcard *.c ./himedia/*.c)
oobjs = $(patsubst %.c,%.o,$(csrcs))

obj-m += hi_osal.o
hi_osal-y := osal_fileops.o osal_vmalloc.o osal_addr.o osal_init.o osal_atomic.o osal_barrier.o osal_cache.o \
             osal_debug.o osal_device.o osal_interrupt.o osal_math.o osal_mutex.o osal_proc.o osal_schedule.o \
             osal_semaphore.o osal_spinlock.o osal_string.o osal_task.o osal_timer.o osal_wait.o osal_workqueue.o \
             osal_notifier.o osal_platform.o ./himedia/base.o ./himedia/himedia.o ./mmz/media_mem.o \
             ./mmz/mmz_userdev.o ./mmz/hisi_allocator.o

hi_osal-$(CONFIG_CMA) += ./mmz/cma_allocator.o
hi_osal-$(CONFIG_CMA) += ./mmz/cmpi_mm.o

all:
	@make -C $(KERNEL_ROOT) M=$(PWD) $(COMPILER_TRIPLE) modules
	@mkdir -p $(REL_KO) && cp hi_osal.ko $(REL_KO)
clean:
	@rm hi_osal.ko -f
	@rm hi_osal.mod.c hi_osal.mod.o hi_osal.o modules.order Module.symvers -f
	@rm $(hi_osal-y) -f
	@rm ./mmz/*.o -f
	@for x in `find . -name "*.mod.c"`;do rm -rf $$x;done
	@for x in `find . -name "*.cmd"`;do rm -rf $$x;done
	@rm .tmp_versions -rf

