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

LOCAL_SRCS := board.c
ARFLAGS = r $$(cp libs/$(RELEASE_TYPE)/lib$(MODULE_NAME).a $@)

include $(MODULE)
