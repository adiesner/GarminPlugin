# -*- mode: Makefile; -*-
# -----------------------------------------
# project GarminPlugin


export PATH := /opt/wx/2.8/bin:$(PATH)
export LD_LIBRARY_PATH := /opt/wx/2.8/lib:$(LD_LIBRARY_PATH)

_WX = /home/gr/projects/gui/codeblocks/wx
_WX.LIB = $(_WX)/lib
_WX.INCLUDE = $(_WX)/include

_CB = /home/gr/projects/gui/codeblocks/cb/src
_CB.INCLUDE = $(_CB)/include
_CB.LIB = $(_CB)/devel



CFLAGS_C = $(filter-out -include "sdk.h",$(CFLAGS))

# -----------------------------------------

# MAKE_DEP = -MMD -MT $@ -MF $(@:.o=.d)

CFLAGS = -Wall 
INCLUDES = 
LDFLAGS =  -s
RCFLAGS = 
LDLIBS = $(T_LDLIBS)  -lstdc++

LINK_exe = gcc -o $@ $^ $(LDFLAGS) $(LDLIBS)
LINK_con = gcc -o $@ $^ $(LDFLAGS) $(LDLIBS)
LINK_dll = gcc -o $@ $^ $(LDFLAGS) $(LDLIBS) -shared
LINK_lib = rm -f $@ && ar rcs $@ $^
COMPILE_c = gcc $(CFLAGS_C) -o $@ -c $< $(MAKEDEP) $(INCLUDES)
COMPILE_cpp = g++ $(CFLAGS) -o $@ -c $< $(MAKEDEP) $(INCLUDES)
COMPILE_rc = windres $(RCFLAGS) -J rc -O coff -i $< -o $@ -I$(dir $<)

%.o : %.c ; $(COMPILE_c)
%.o : %.cpp ; $(COMPILE_cpp)
%.o : %.cxx ; $(COMPILE_cpp)
%.o : %.rc ; $(COMPILE_rc)
.SUFFIXES: .o .d .c .cpp .cxx .rc

all: all.before all.targets all.after

all.before :
	-
all.after : $(FIRST_TARGET)
	
all.targets : Release_target Debug_target 

clean :
	rm -fv $(clean.OBJ)
	rm -fv $(DEP_FILES)

.PHONY: all clean distclean

# -----------------------------------------
# Release_target

Release_target.BIN = bin/Release/npGarminPlugin.so
Release_target.OBJ = configManager.o deviceManager.o edge305Device.o edge705Device.o garminFilebasedDevice.o gpsDevice.o log.o main.o messageBox.o oregonDevice.o sdCardDevice.o 
DEP_FILES += configManager.d deviceManager.d edge305Device.d edge705Device.d garminFilebasedDevice.d gpsDevice.d log.d main.d messageBox.d oregonDevice.d sdCardDevice.d 
clean.OBJ += $(Release_target.BIN) $(Release_target.OBJ)

Release_target : Release_target.before $(Release_target.BIN) Release_target.after_always
Release_target : CFLAGS += -O2 -fPIC  -Os
Release_target : INCLUDES += -I/usr/lib/xulrunner-devel-1.9.1.7/include -I../ticpp -I/usr/lib/xulrunner-devel-1.9.1.8/include -I../garmintools/src 
Release_target : RCFLAGS += 
Release_target : LDFLAGS += -s  $(CREATE_LIB) $(CREATE_DEF)
Release_target : T_LDLIBS = ../ticpp/lib/libticppd.a ../garmintools/src/.libs/libgarmintools.a /usr/lib64/libusb.so 
ifdef LMAKE
Release_target : CFLAGS -= -O1 -O2 -g -pipe
endif

Release_target.before :
	
	
Release_target.after_always : $(Release_target.BIN)
	
$(Release_target.BIN) : $(Release_target.OBJ)
	$(LINK_dll)
	

# -----------------------------------------
# Debug_target

Debug_target.BIN = bin/Debug/npGarminPlugin.so
Debug_target.OBJ = configManager.o deviceManager.o edge305Device.o edge705Device.o garminFilebasedDevice.o gpsDevice.o log.o main.o messageBox.o oregonDevice.o sdCardDevice.o 
DEP_FILES += configManager.d deviceManager.d edge305Device.d edge705Device.d garminFilebasedDevice.d gpsDevice.d log.d main.d messageBox.d oregonDevice.d sdCardDevice.d 
clean.OBJ += $(Debug_target.BIN) $(Debug_target.OBJ)

Debug_target : Debug_target.before $(Debug_target.BIN) Debug_target.after_always
Debug_target : CFLAGS += -g -fPIC  -Os
Debug_target : INCLUDES += -I/usr/lib/xulrunner-devel-1.9.1.8/include -I../ticpp -I../garmintools/src 
Debug_target : RCFLAGS += 
Debug_target : LDFLAGS +=  $(CREATE_LIB) $(CREATE_DEF)
Debug_target : T_LDLIBS = ../ticpp/lib/libticppd.a ../garmintools/src/.libs/libgarmintools.a /usr/lib64/libusb.so 
ifdef LMAKE
Debug_target : CFLAGS -= -O1 -O2 -g -pipe
endif

Debug_target.before :
	
	
Debug_target.after_always : $(Debug_target.BIN)
	
$(Debug_target.BIN) : $(Debug_target.OBJ)
	$(LINK_dll)
	

# -----------------------------------------
ifdef MAKE_DEP
-include $(DEP_FILES)
endif
