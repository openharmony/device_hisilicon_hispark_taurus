# Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
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

include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := bsp_config

ifeq ($(LOSCFG_DEBUG_VERSION),y)
	RELEASE_TYPE := debug
else
	RELEASE_TYPE := release
endif

DIR = temp
LIB = lib$(MODULE_NAME).a

SRCS = board.c
OBJS = $(patsubst %.c,$(DIR)/%.o,$(SRCS))

$(OBJS): $(DIR)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

COBJS = $(OBJS)

$(COBJS): $(DIR)

$(DIR):
	@mkdir -p $@

$(LIB): $(COBJS)
	@$(AR) -x -o libs/$(RELEASE_TYPE)/$@
	@mv *.o $(DIR)
	@$(AR) cr $(DIR)/$@ $(DIR)/*.o

all: $(LIB)
	@if [ ! -d $(OUT)/lib ]; then mkdir -p $(OUT)/lib; fi
	@cp -rf $(DIR)/$(LIB) $(OUT)/lib

clean:
	@rm -rf $(OUT)$(LIB)
	@rm -rf $(DIR)

.PHONY: all clean
