#
#  Copyright (c) 2019    European Spallation Source ERIC
#
#  The program is free software: you can redistribute
#  it and/or modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation, either version 2 of the
#  License, or any newer version.
#
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU General Public License along with
#  this program. If not, see https://www.gnu.org/licenses/gpl-2.0.txt
#
# 
# Author  : anderssandstrom
# email   : anderssandstrom@esss.se
# Date    : 2020Mar22-1607-33CET
# version : 0.0.0 
#
# template file is generated by ./e3TemplateGenerator.bash with bf03d40
# Please look at many other _module_.Makefile in e3-* repository
# 

## The following lines are mandatory, please don't change them.
where_am_I := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include $(E3_REQUIRE_TOOLS)/driver.makefile
include $(E3_REQUIRE_CONFIG)/DECOUPLE_FLAGS

ifneq ($(strip $(ASYN_DEP_VERSION)),)
asyn_VERSION=$(ASYN_DEP_VERSION)
endif

ifneq ($(strip $(ECMC_DEP_VERSION)),)
ecmc_VERSION=$(ECMC_DEP_VERSION)
endif

ifneq ($(strip $(EXPRTK_DEP_VERSION)),)
exprtk_VERSION=$(EXPRTK_DEP_VERSION)
endif

APP:=ecmc_plugin_pvaApp
APPDB:=$(APP)/Db
APPSRC:=$(APP)/src

USR_CFLAGS   += -shared -fPIC -Wall -Wextra
#USR_CPPFLAGS += -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 
USR_CPPFLAGS += -std=c++11
USR_LDFLAGS  += -lstdc++
USR_LDFLAGS  += -L$(EPICS_BASE)/lib/$(EPICS_HOST_ARCH)
USR_LDFLAGS  += -lpvaClient 
USR_LDFLAGS  += -lpvAccess 
USR_LDFLAGS  += -lpvAccessCA
USR_LDFLAGS  += -lnt
USR_LDFLAGS  += -lpvData
USR_LDFLAGS  += -lca
USR_LDFLAGS  += -lCom

$(info $$USR_LDFLAGS is [${E3_LD_LIBRARY_PATH}])
USR_INCLUDES += -I$(where_am_I)$(APPSRC)

TEMPLATES += $(wildcard $(APPDB)/*.db)
TEMPLATES += $(wildcard $(APPDB)/*.template)
SOURCES += $(APPSRC)/ecmcPluginPva.c
SOURCES += $(APPSRC)/ecmcPvaWrap.cpp
SOURCES += $(APPSRC)/ecmcPv.cpp

db:

.PHONY: db

vlibs:

.PHONY: vlibs

###
