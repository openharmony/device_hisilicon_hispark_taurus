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

# Enum the C files needed to be compiled, using the relative path
HIFB_SRC += src/hifb_main.c

ifdef CONFIG_HI_PROC_SHOW_SUPPORT
HIFB_SRC += src/hifb_proc.c
endif
ifeq ($(OSTYPE),linux)
    ifeq ($(HIARCH), hi3559av100)
        HIFB_SRC += src/hifb_dmabuf.c
    endif
    HIFB_SRC += src/init/linux/hifb_init.c
else ifeq ($(OSTYPE), liteos)
    HIFB_SRC += src/init/liteos/hifb_init.c
endif
