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


local_dir := kernel

.PHONY: all clean $(local_dir) $(clean_dir)

all: $(local_dir)

$(local_dir):
	cd $@; make || exit1;

clean_dir:= $(foreach dir, $(local_dir), $(patsubst %, %_clean, $(dir)))

clean: $(clean_dir)

$(clean_dir):
	@cd $(patsubst %_clean, %, $@) ; make clean;

