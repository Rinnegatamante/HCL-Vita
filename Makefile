TARGET		:= HCL
TITLE		:= HYDCASLAB
VPK_TITLE	:= Hydra Castle Labyrinth
SOURCES		:= source source/enemies source/vita

LIBS = -lvita2d -lSceLibKernel_stub -lScePvf_stub -lSceAppMgr_stub -lSceCtrl_stub \
	-lSceSysmodule_stub -lm -lSceAppUtil_stub -lScePgf_stub -ljpeg -lfreetype -lc -lScePower_stub \
	-lSceCommonDialog_stub -lpng16 -lz -lspeexdsp -lSceAudio_stub -lSceGxm_stub -lSceDisplay_stub \

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -fno-lto -g -Wl,-q -O2 -D_PSP2 -DHAVE_LIBSPEEXDSP \
		-DWANT_FMMIDI=1 -DUSE_AUDIO_RESAMPLER -DWANT_FASTWAV
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11 -fpermissive
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself -s $< build\eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE) "$(VPK_TITLE)" param.sfo
	cp -f param.sfo build/sce_sys/param.sfo
	
	#------------ Comment this if you don't have 7zip ------------------
	7z a -tzip $(TARGET).vpk -r .\build\sce_sys\* .\build\eboot.bin 
	#-------------------------------------------------------------------

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself -s $@ eboot.bin

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) eboot.bin