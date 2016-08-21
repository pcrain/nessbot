RM := rm -rf
MKDIR_P = mkdir -p

UNUSED := -Wno-unused-variable

OBJS += \
build/nessbot.o \
build/neural.o \
build/termoutput.o \
build/deviceio.o \
build/memreader.o \
build/util.o \
build/main.o

CPP_DEPS += \
build/nessbot.d \
build/neural.d \
build/termoutput.d \
build/deviceio.d \
build/memreader.d \
build/util.d \
build/main.d

LIBS := -ljsoncpp -lncurses

OUT_DIR = build

all: directories nessbot

nessbot: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L/usr/lib -std=c++11 -fopenmp -o "./cnessbot" $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

build/%.o: ./src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__GXX_EXPERIMENTAL_CXX0X__ -I/usr/include/jsoncpp -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 $(UNUSED) -fopenmp -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

clean:
	-$(RM) $(OBJS)$(C++_DEPS) ./cnessbot
	-@echo ' '

directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}

.PHONY: all clean dependents directories
.SECONDARY:
