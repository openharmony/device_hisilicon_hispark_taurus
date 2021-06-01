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

# cbb global variables definition
include Makefile.param

.PHONY:clean all base sys vi mcf vo vo_dev vpss avs chnl region vgs gdc venc vedu vdec vfmw jpegd dis isp audio audio_bin

target = base sys

# cbb common include file
HI_COMM_INC_FILE += $(CBB_ROOT)/dis/gmedis/include/hi_comm_dis.h
HI_COMM_INC_FILE += $(CBB_ROOT)/region/include/hi_comm_region.h
HI_COMM_INC_FILE += $(VGS_ROOT)/include/hi_comm_vgs.h
HI_COMM_INC_FILE += $(CBB_ROOT)/sys/include/hi_comm_sys.h
HI_COMM_INC_FILE += $(VENC_ROOT)/include/h7/hi_comm_venc.h
HI_COMM_INC_FILE += $(VENC_ROOT)/include/h7/hi_comm_rc.h
HI_COMM_INC_FILE += $(CBB_ROOT)/vpss/include/hi_comm_vpss.h
HI_COMM_INC_FILE += $(CBB_ROOT)/include/hi_common.h
HI_COMM_INC_FILE += $(CBB_ROOT)/include/hi_comm_video.h
HI_COMM_INC_FILE += $(CBB_ROOT)/base/include/hi_comm_vb.h
HI_COMM_INC_FILE += $(CBB_ROOT)/vi/include/hi_comm_vi.h
HI_COMM_INC_FILE += $(CBB_ROOT)/vi/include/hi_comm_snap.h
HI_COMM_INC_FILE += $(CBB_ROOT)/vo/include/hi_comm_vo.h
HI_COMM_INC_FILE += $(CBB_ROOT)/vo/vo_dev/include/hi_comm_vo_dev.h
HI_COMM_INC_FILE += $(CBB_ROOT)/gdc/include/hi_comm_gdc.h

ifeq ($(CONFIG_HI_VO_SUPPORT), y)
    target += vo_dev
    MPI_SRC_DIR += $(CBB_ROOT)/vo/vo_dev/mpi
    MPI_CFLAGS += -I$(CBB_ROOT)/vo/vo_dev/mpi/adapt
    MPI_CFLAGS += -I$(CBB_ROOT)/vo/vo_dev/include/adapt
    MPI_INC_DIR += $(CBB_ROOT)/vo/vo_dev/include
endif

########################################################

#-----------------------------------------------------

target_clean = $(patsubst %,%_clean,$(target))
#-----------------------------------------------------

#-----------------------------------------------------
all: $(target)

mpi: $(MPI_OBJS)
	@mkdir -p "$(REL_LIB)"
	@mkdir -p "$(REL_INC)"
	@mkdir -p "$(ADAPT_INC)"
	@$(AR) -rsv $(LIB_A) $(MPI_OBJS) > /dev/null
	@$(CC) $(LIBS_LD_CFLAGS) -fPIC -shared -o $(LIB_S) $(MPI_OBJS) > /dev/null
	@$(call maco_cp_file,$(INC_FILE),$(REL_INC))
	@$(call maco_cp_file,$(HI_COMM_INC_FILE),$(REL_INC))
	@$(call maco_cp_file,$(ADPAT_FILE),$(ADAPT_INC))

base:
	@cd based && make

sys:
	@cd sysd && make

chnl:
	@cd chnl && make

region:
	@cd region && make

vgs:
	@cd $(VGS_ROOT) && make

vi:
	@cd vi && make

mcf:
	@cd $(MCF_ROOT) && make
vo:
	@cd vo && make
vo_dev:
	@cd vo/vo_dev && make
vpss:
	@cd $(VPSS_ROOT) && make
avs:
	@cd avs && make
vdec:
	@cd $(VDEC_ROOT) && make
vfmw:
	@cd $(VFMW_ROOT) && make
jpegd:
	@cd jpegd && make

gdc:
	@cd gdc && make
venc:
	@cd $(VENC_ROOT) && make
vedu:
	@cd vedu && make
dis:
	@cd dis && make

clean:$(target_clean)
	@rm -fr $(OBJ_DIR) $(LIB_A) $(LIB_S)
	@$(call maco_rm,$(REL_FILE),$(REL_INC))
	@$(call maco_rmemptydir,$(ADAPT_INC))
	@$(call maco_rmemptydir,$(REL_INC))
	@$(call maco_rmemptydir,$(REL_LIB))
mpi_clean:
	@rm -fr $(MPI_OBJS) $(LIB_A) $(LIB_S)
base_clean:
	@cd based && make clean
sys_clean:
	@cd sysd && make clean
chnl_clean:
	@cd chnl && make clean
venc_clean:
	@cd $(VENC_ROOT) && make clean
vedu_clean:
	@cd vedu && make clean
region_clean:
	@cd region && make clean
vgs_clean:
	@cd $(VGS_ROOT) && make clean
vi_clean:
	@cd vi && make clean
mcf_clean:
	@cd $(MCF_ROOT) && make clean
vo_clean:
	@cd vo && make clean
vo_dev_clean:
	@cd vo/vo_dev && make clean
vpss_clean:
	@cd $(VPSS_ROOT) && make clean
avs_clean:
	@cd avs && make clean
vdec_clean:
	@cd $(VDEC_ROOT) && make clean
vfmw_clean:
	@cd $(VFMW_ROOT) && make clean
jpegd_clean:
	@cd jpegd && make clean
gdc_clean:
	@cd gdc && make clean
dis_clean:
	@cd dis && make clean
