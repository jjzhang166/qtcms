
TARGET := onvif.out

ECHO := @echo
LINK := ln -s
CP := cp -av
MKDIR := @mkdir -p
MV := mv -v
MAKE := make
PRINTF := @printf "\033[32;1m\t%s\033[0m\n"
RM := /bin/rm

#toolchain define
PRJ_CROSS?=
#PRJ_CROSS?=arm-hisiv100nptl-linux-
CC    := $(PRJ_CROSS)gcc
CXX   := $(PRJ_CROSS)g++
AR    := $(PRJ_CROSS)ar
STRIP := $(PRJ_CROSS)strip

#dir define
BUILD ?= debug
sc_opt=
# project , normal:jamedia:ipctester
PROJECT ?= normal

TMP_DIR:=$(CURDIR)/tmp/$(BUILD)
LIB_DIR:=$(CURDIR)/lib/$(BUILD)
INSTALL_DIR :=$(CURDIR)/install/$(BUILD)

#src/obj/dep/lib(app only) define
SRC_TMP := $(wildcard *.c) $(wildcard *.cpp)
SRC := $(foreach n,$(SRC_TMP), $(CURDIR)/$(n))

OBJ_TMP := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRC_TMP)))
OBJ := $(foreach n,$(OBJ_TMP), $(TMP_DIR)/$(n))

DEP_TMP := $(patsubst %.c,%.d,$(SRC_TMP))
DEP := $(foreach n,$(DEP_TMP), $(TMP_DIR)/$(n))


LIB_TMP := onvif generic
LIB := $(foreach n,$(LIB_TMP), $(LIB_DIR)/lib$(n).a)
LIB_INC := $(foreach n,$(LIB_TMP), -I$(CURDIR)/$(n) -I$(CURDIR)/include/$(n) )


INCS := -I$(CURDIR) \
		-I$(CURDIR)/include \
		$(LIB_INC)

DEFS :=-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -D_GNU_SOURCE -DLINUX -DTRUE=1 -DFALSE=0
#DEFS +=-D_IPC_TESTER
DEFS +=-DONVIF_TEST

#cflags/ldflags(app only) define
EXTRA_CFLAGS := -Wno-unused-but-set-variable -Wno-unused-function -Wno-format-zero-length
CFLAGS := -Wall -O0 -g3 $(EXTRA_CFLAGS)
# CFLAGS += -std=c99
#CFLAGS += -DWITH_IPV6
#CFLAGS += -DDEBUG
LDFLAGS := $(LIBS) -lpthread -lm


# handle param
ifeq ($(BUILD), debug)
DEFS += -DDEBUG
else
endif


ifeq ($(PROJECT), normal)
#DEFS += -DHAVE_ARP
endif
ifeq ($(PROJECT), jamedia)
DEFS += -DNVP_VENC_NR=4
DEFS += -DHAVE_ARP
endif
ifeq ($(PROJECT), ipctester)
else
DEFS += -DHAVE_IFADDRS_H
#DEFS += -DHAVE_ARP
endif

#special target define
.PHONY: clean prepare $(LIB) all


all : $(LIB) prepare
	$(PRINTF) "compile done!!!!!!!!!!!!!!!!"

demo : $(TARGET)
	$(PRINTF) "compile done!!!!!!!!!!!!!!!!"

prepare:
	@if [ ! -d $(LIB_DIR) ];then mkdir -p $(LIB_DIR);fi
	@if [ ! -d $(TMP_DIR) ];then mkdir -p $(TMP_DIR);fi


$(TARGET): $(OBJ) $(LIB)
	$(PRINTF) "LD  $@";
	$(CC) $(OBJ) $(LIB) $(LDFLAGS) -o $@

install: prepare
	$(PRINTF) "Make Install ONVIF"
	$(MKDIR) $(INSTALL_DIR)/lib
	$(MKDIR) $(INSTALL_DIR)/include
	$(CP) $(LIB_DIR)/*.a $(INSTALL_DIR)/lib/
	$(CP) $(CURDIR)/onvif/onvif.h $(INSTALL_DIR)/include/
ifeq ($(BUILD), release)
	#$(STRIP) $(INSTALL_DIR)/lib/*.a
endif
	@ls -lh $(INSTALL_DIR)/lib/*.a

$(LIB):
	$(MAKE) -f ../Makefile.lib PRJ_CROSS="$(PRJ_CROSS)" DEFS="$(DEFS)" INCS="$(INCS)" CFLAGS="$(CFLAGS)" BUILD="$(BUILD)" -C $(subst .a,,$(subst $(LIB_DIR)/lib,,$@))

#common target define
ifneq ($(MAKECMDGOALS), clean)
-include $(DEP)
endif

clean:
	$(RM) -rf $(TMP_DIR)
	$(RM) -rf $(LIB_DIR)
	$(RM) -rf $(TARGET)

clean_all: clean
	$(RM) -f onvif/soap*
	$(RM) -f *.log
	$(RM) -rf install
	$(RM) -rf lib
	$(RM) -rf tmp

$(TMP_DIR)/%.d: %.c
	$(MKDIR) $(TMP_DIR)
	$(PRINTF) "DEP $@";$(CC) -MM -MF"$@" -MT"$(@:.d=.o)" $(INCS) $(DEFS) $<

$(TMP_DIR)/%.o: %.c
	$(MKDIR) $(TMP_DIR)
	$(PRINTF) "CC  $@";$(CC) -c $(CFLAGS) $(INCS) $(DEFS) -o "$@" $<

$(TMP_DIR)/%.d: %.cpp
	$(MKDIR) $(TMP_DIR)
	$(PRINTF) "DEP $@";$(CXX) -MM -MF"$@" -MT"$(@:.d=.o)" $(INCS) $(DEFS) $<

$(TMP_DIR)/%.o: %.cpp
	$(MKDIR) $(TMP_DIR)
	$(PRINTF) "CXX  $@";
	$(CXX) -c $(CFLAGS) $(INCS) $(DEFS) -o "$@" $<

