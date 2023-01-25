
LINKERSCRIPTS    := linkerscripts

#---------------------------------------------------------------------------------
#.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#---------------------------------------------------------------------------------

# NOTE: TARGET and BUILD are now passed from parent Makefile
TARGET		?=	$(notdir $(CURDIR))
BUILD		?=	build
SOURCES		:= 	source $(filter-out %.c %.cpp %.s,$(wildcard source/* source/*/* source/*/*/* source/*/*/*/*))
DATA		:=	data
INCLUDES	:=	include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec

# change to O3 or even Ofast if nothing breaks for final release
# also try enabling LTO to get max perf
CFLAGS	:=	-g -O3 -flto -Wall -Wno-multichar -ffunction-sections \
			$(ARCH) $(DEFINES)

CFLAGS	+=	$(INCLUDE) -D__SWITCH__ -DNOLOG

CXXFLAGS	:= $(PORTAL) $(CFLAGS) -std=gnu++20 -fpermissive -fno-rtti -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -enable-libstdcxx-allocator=new -fpermissive 

ASFLAGS	:=	-g $(ARCH)
LDFLAGS  =  -specs=../switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map),--version-script=$(TOPDIR)/exported.txt -Wl,-init=__custom_init -Wl,-fini=__custom_fini -Wl,--export-dynamic -nodefaultlibs

LIBS	:= -lstdc++ -u malloc

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(LIBNX)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT		:=	$(CURDIR)/$(TARGET)
export TOPDIR		:=	$(CURDIR)
export OUTPUTNSO	:=	$(CURDIR)/subsdk9

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	?=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) \
			-I$(PORTLIBS)/include/freetype2

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.jpg)
	ifneq (,$(findstring $(TARGET).jpg,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).jpg
	else
		ifneq (,$(findstring icon.jpg,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.jpg
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
	export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
	export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------

all: $(BUILD)

$(BUILD):
	@echo $(LDFLAGS)
	@echo "${SOURCES}"
	@echo "${CPPFILES}"
	@echo "${CXXFLAGS}"
	@[ -d $@ ] || mkdir -p $@
	$(MAKE) -C $(BUILD) -f $(CURDIR)/$(MAKE_NSO)

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr build* *.nso *.elf subsdk9


#---------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
%.nso: %.elf
	elf2nso $< $(OUTPUTNSO)
	@echo built ... $(notdir $@)

%.nro: %.elf
	@elf2nro $< $@ $(NROFLAGS)
	@echo built ... $(notdir $@)

#---------------------------------------------------------------------------------
all	: $(OUTPUT).nso

$(OUTPUT).elf   : $(OFILES)


$(OFILES_SRC)	: $(HFILES_BIN)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
