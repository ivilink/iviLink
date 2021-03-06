# 
# iviLINK SDK, version 1.0.1
# http://www.ivilink.net
# Cross Platform Application Communication Stack for In-Vehicle Applications
# 
# Copyright (C) 2012, Luxoft Professional Corp., member of IBS group
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; version 2.1.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
# 
# 
ifndef CONFIG_MK_EXPORTED
ifdef AXIS_CONFIG_MK
include $(AXIS_CONFIG_MK)
else
$(error config.mk is not exported and AXIS_CONFIG_MK variable cannot be found)
endif
endif
ifndef COMMON_MK
$(error COMMON_MK is not defined)
endif
include $(COMMON_MK)

###############################################################################
# Subfolders
define SUBFOLDERS
endef
###############################################################################
# This target

SRC_DIR := .
SRC := 

OUT_FNAME     := libIviLinkApp.so

INCLUDES      := 
CXXFLAGS      := $(GLOBAL_CXXFLAGS) $(INCLUDES)
LDFLAGS       := $(GLOBAL_LDFLAGS)

LIBS := 
LIBS += framework/public/appLib/libApp.a
LIBS += framework/public/profileLib/libProfileLib.a
LIBS += framework/libraries/ServiceManager/libServices.a
LIBS += framework/libraries/AppMan/App/libAppManApp.a
LIBS += framework/components/ProfileManager/PMAL/libPMAL.a
LIBS += framework/components/ProfileManager/PMAL/core/libPMAL_core.a
LIBS += framework/components/ProfileManager/PMAL/PIM/libPMAL_PIM.a
LIBS += framework/components/ProfileManager/PMAL/ipc_protocol/libPMAL_IPC.a
LIBS += framework/components/ProfileManager/IpcProtocol/libPM_IpcProtocol.a

DLIBS :=
DLIBS += utils/shared_lib/libIviLinkUtils.so
DLIBS += framework/components/ChannelSupervisor/ChannelLib/libIviLinkChannel.so

LIBS := $(call GET_DEPS,$(LIBS))

###############################################################################
# Variables

OUT_PATH := $(call OUT_PATH_FUNC)
$(info OUT_PATH $(OUT_PATH))

OUT_FULLDIR   := $(OUT_PATH)
OUT_FULLFNAME := $(OUT_FULLDIR)$(OUT_FNAME)


###############################################################################
# Rules
$(OUT_FULLDIR):
	mkdir -p $@

$(OUT_FULLFNAME): $(LIBS) $(call GET_DEPS,$(DLIBS)) Makefile | $(OUT_FULLDIR) 
	$(CXX) -shared -Wl,--whole-archive $(LIBS) -Wl,--no-whole-archive $(call GET_LIB_LDFLAGS,,$(DLIBS)) $(LDFLAGS) -o $@

debug: $(OUT_FULLFNAME)
	$(SUBFOLDERS)

release: $(OUT_FULLFNAME)
	$(SUBFOLDERS)

clean:
	-rm -rf $(OUT_FULLDIR)
	$(SUBFOLDERS)

.PHONY: debug release clean
 

ifeq (,$(findstring $(MAKECMDGOALS),dist clean depend doc docs))
-include $(DEPS)
endif
