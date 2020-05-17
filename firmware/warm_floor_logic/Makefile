TOOLROOT=/usr/local/Cellar/gcc-arm-none-eabi/20160928/bin

CC=$(TOOLROOT)/arm-none-eabi-gcc
LD=$(TOOLROOT)/arm-none-eabi-g++
AR=$(TOOLROOT)/arm-none-eabi-ar
AS=$(TOOLROOT)/arm-none-eabi-as
OBJCOPY=$(TOOLROOT)/arm-none-eabi-objcopy
# define the C compiler to use

MAIN := warm_floor
BUILD_DIR := build

DEFINES = -DUSE_FULL_ASSERT -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F10X_LD_VL -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -DSTM32F10X  -Wstack-usage=256
CFLAGS =  -Wall -Wextra -Og -ffreestanding -mcpu=cortex-m3 -mthumb -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -g3
INCLUDES = -I./include -I./system/include -I./system/include/cmsis -I./system/include/stm32f1-stdperiph -I./src/drivers -I./libs -I./protobuf -I./Middlewares/Third_Party/FreeRTOS/Source/include -I./Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I./Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
LFLAGS = -std=gnu11  -T mem.ld -T libs.ld -T sections.ld -nostartfiles -Xlinker --gc-sections -L"./ldscripts" -Wl,-Map,"$(BUILD_DIR)/$(MAIN).map" --specs=nano.specs 
LIBS = 
FREERTOS = Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c Middlewares/Third_Party/FreeRTOS/Source/queue.c Middlewares/Third_Party/FreeRTOS/Source/tasks.c Middlewares/Third_Party/FreeRTOS/Source/list.c
PROTOBUF = protobuf/floor.pb.c protobuf/pb_decode.c protobuf/pb_common.c protobuf/pb_encode.c
STM32STDPERIF = system/src/stm32f1-stdperiph/stm32f10x_usart.c system/src/stm32f1-stdperiph/stm32f10x_bkp.c system/src/stm32f1-stdperiph/stm32f10x_rcc.c system/src/stm32f1-stdperiph/stm32f10x_gpio.c system/src/stm32f1-stdperiph/stm32f10x_iwdg.c system/src/stm32f1-stdperiph/stm32f10x_tim.c system/src/stm32f1-stdperiph/stm32f10x_adc.c system/src/stm32f1-stdperiph/stm32f10x_dma.c system/src/stm32f1-stdperiph/stm32f10x_spi.c system/src/stm32f1-stdperiph/stm32f10x_exti.c  system/src/stm32f1-stdperiph/misc.c
SYSTEM = system/src/cmsis/system_stm32f10x.c system/src/cmsis/vectors_stm32f10x.c system/src/cortexm/_initialize_hardware.c system/src/cortexm/_reset_hardware.c system/src/cortexm/exception_handlers.c
NEWLIB = system/src/newlib/_exit.c system/src/newlib/_sbrk.c system/src/newlib/_startup.c system/src/newlib/_syscalls.c system/src/newlib/assert.c
DRIVERS = src/drivers/DS18B20.c src/drivers/ssd1306.c src/drivers/fonts.c src/drivers/nRF24L01.c src/drivers/onewire.c src/drivers/bus_spi.c

SRCS = src/main.c src/common.c src/led.c src/display.c src/radio.c src/serial.c src/switch.c src/thermometer.c libs/libdproto.c $(PROTOBUF) $(FREERTOS) $(STM32STDPERIF) $(DRIVERS) $(SYSTEM) $(NEWLIB)
OBJS = $(addprefix build/,$(SRCS:.c=.o))


$(MAIN): $(BUILD_DIR) $(OBJS)
	$(LD) $(CFLAGS) $(LFLAGS) -o $(BUILD_DIR)/$(MAIN).elf $(OBJS) $(LIBS)
	$(OBJCOPY) $(BUILD_DIR)/$(MAIN).elf -O ihex $(BUILD_DIR)/$(MAIN).hex 
all: $(MAIN)
	@echo $(MAIN) build completed

# -MMD -MP -MF"$(<:.c=.d)" -MT"$<"


$(BUILD_DIR)/%.o: %.c
	@echo Processing $(@:$(BUILD_DIR)/%=%)
	mkdir -p $(BUILD_DIR)/$(dir $(@:$(BUILD_DIR)/%=%))
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	@echo Creating build direcotry
	mkdir $@
	
clean:
	$(RM) *.o *~ $(MAIN) $(MAIN).hex $(MAIN).map 
	$(RM) -R $(BUILD_DIR)

.PHONY: clean