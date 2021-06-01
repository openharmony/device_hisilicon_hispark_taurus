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

# $(ROOT)/mpp/Makefile

-include $(shell pwd)/../.config

ifeq ($(PARAM_FILE),)
    PARAM_FILE:=./Makefile.param
    include $(PARAM_FILE)
endif



.PHONY:all prepare cbb component drv vendor clean

sub_dir := prepare cbb component

ifeq ($(CONFIG_INTERDRV), y)
    sub_dir += interdrv
endif


all: $(sub_dir)

prepare:
	@cd $(SDK_PATH)/osal/$(OSTYPE) && make

interdrv:
	@cd $(INTERDRV_ROOT) && make


cbb:
	@cd cbb && make

component:
	@cd component && make


clean:
ifneq ($(OSTYPE),liteos)
#	@cd $(SDK_PATH)/mpp/component/securec/src && make clean
endif
	@cd $(SDK_PATH)/osal/$(OSTYPE) && make clean
ifeq ($(CONFIG_INTERDRV), y)
	@cd $(INTERDRV_ROOT) && make clean
endif
	@cd component && make clean
	@cd cbb && make clean
	@rm -rf $(REL_KO)/*.ko
	@rm -rf $(REL_OBJ)/*.o
	@rm $(REL_KO)/extdrv -fr
