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

# $(ROOT)/mpp/component/Makefile

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(HIARCH),hi3516ev200)
        TDE_ROOT    := tde_hi3516ev200
        PM_ROOT     := pm3
else
        TDE_ROOT    := tde
        PM_ROOT     := pm2
endif

ifeq ($(CONFIG_HI_TDE_SUPPORT), y)
components += tde
endif

ifeq ($(CONFIG_HI_HDMI_SUPPORT),y)
components += hdmi
endif

ifeq ($(CONFIG_HI_HIFB_SUPPORT),y)
components += hifb
endif

components_clean := $(foreach dir, $(components), $(patsubst %, %_clean, $(dir)))

.PHONY:clean all $(components) $(components_clean)

all: $(components)
	@echo -e "\e[0;32;1m--Compiling 'component'... Configs as follow:\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), HIDBG=$(HIDBG)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@echo ---- KERNEL_ROOT=$(KERNEL_ROOT)
	@echo -e "\e[0m"

clean: $(components_clean)
	@echo -e "\e[0;32;1m Clean ok! \e[0m"

tde:
	@cd $(TDE_ROOT) && make
tde_clean:
	@cd $(TDE_ROOT) && make clean

hdmi:
	@cd hdmi/src && make
hdmi_clean:
	@cd hdmi/src && make clean

hifb:
	@cd hifb && make
hifb_clean:
	@cd hifb && make clean

avs_lut:
	@cd avs_lut && make
avs_lut_clean:
	@cd avs_lut && make clean

pos_query:
	@cd pos_query && make
pos_query_clean:
	@cd pos_query && make clean

pciv:
	@cd pci && make
pciv_clean:
	@cd pci && make clean

fisheye_adjust:
	@cd fisheye_adjust && make
fisheye_adjust_clean:
	@cd fisheye_adjust && make clean
vcmp:
	@cd vcmp && make
vcmp_clean:
	@cd vcmp && make clean

photo:
	@cd photo && make
photo_clean:
	@cd photo && make clean

svp:
	@cd svp && make
svp_clean:
	@cd svp && make clean

pm:
	@cd $(PM_ROOT) && make
pm_clean:
	@cd $(PM_ROOT) && make clean

avs_transcode:
	@cd avc_transcode && make
avs_transcode_clean:
	@cd avc_transcode && make clean

tzasc:
	@cd tzasc && make
tzasc_clean:
	@cd tzasc && make clean

motionfusion:
	@cd motionfusion && make
motionfusion_clean:
	@cd motionfusion && make clean

mcf_calibration:
	@cd mcf_calibration && make
mcf_calibration_clean:
	@cd mcf_calibration && make clean

hisyslink:
	@cd hisyslink && make
hisyslink_clean:
	@cd hisyslink; make clean

