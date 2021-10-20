# Copyright (c) 2021 HiSilicon (Shanghai) Technologies CO., LIMITED.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(PARAM_FILE), )
    PARAM_FILE:= ../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

SYSROOT_PATH := $(OHOS_ROOT_PATH)/out/ohos-arm-release/obj/third_party/musl
LLVM_SYSROOT_CFLAGS := --sysroot=$(SYSROOT_PATH)
LLVM_TARGET_CFLAGS := -target arm-linux-ohosmusl
LIBS_CFLAGS += $(LLVM_SYSROOT_CFLAGS) -fuse-ld=lld $(LLVM_TARGET_CFLAGS)
LIBS_LD_CFLAGS +=$(LLVM_SYSROOT_CFLAGS) -fuse-ld=lld $(LLVM_TARGET_CFLAGS)
LIBS_LD_CFLAGS += -L$(SYSROOT_PATH)/usr/lib/arm-linux-ohosmusl

csrcs = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(csrcs))

LIBS = libsns_sc4210.so

CFLAGS += -I$(PWD)
CFLAGS += -I$(PWD)/../include
CFLAGS += -I$(PWD)/../../../mpp/cbb/include
CFLAGS += -I$(PWD)/../../../mpp/cbb/based/arch/hi3516cv500/include/hi3516cv500
CFLAGS += $(MPP_CFLAGS)
CFLAGS += $(LIBS_CFLAGS)

all: $(OBJS) $(LIBS)

$(OBJS): %.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(LIBS): $(OBJS)
	@$(CC) $(LIBS_LD_CFLAGS) -shared -o $(LIBS) $(OBJS)
	@$(RM) $(OBJS)

clean:
	@$(RM) $(OBJS) $(LIBS)
