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
    PARAM_FILE:= ../mpp/Makefile.param
    include $(PARAM_FILE)
endif

.PHONY:clean all interdrv common

sub_dir := aa
sub_dir += common
ifeq ($(CONFIG_INTERDRV), y)
    sub_dir += interdrv
endif

all: $(sub_dir)

interdrv:
	@cd $(INTER_DRV) && make
common:
	@cd common && make
clean:
	@cd common && make clean
	@cd $(INTER_DRV) && make clean
aa:
	echo "$(CONFIG_HI_CHIP_TYPE)"
