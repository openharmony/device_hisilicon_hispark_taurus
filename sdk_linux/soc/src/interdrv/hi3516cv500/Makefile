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

sub_dir:= 

ifeq ($(CONFIG_MIPI_RX), y)
    sub_dir += mipi_rx
endif

ifeq ($(CONFIG_MIPI_TX), y)
    sub_dir += mipi_tx
endif

ifeq ($(CONFIG_HI_SYSCFG), y)
    sub_dir += sysconfig
endif

clean_dir:= $(foreach dir, $(sub_dir), $(patsubst %, %_clean, $(dir)))

.PHONY: all clean $(sub_dir) $(clean_dir)

all: $(sub_dir)

$(sub_dir):
	@cd $@; make || exit1;

clean: $(clean_dir)

$(clean_dir):
	@cd $(patsubst %_clean, %, $@) ; make clean;
