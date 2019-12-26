CC=$(CROSS_COMPILE)gcc
OBJCOPY=$(CROSS_COMPILE)objcopy
STRIP=$(CROSS_COMPILE)strip

override CFLAGS+=-Os -fstack-protector-strong -fno-strict-aliasing  -mcpu=cortex-m4  -mfloat-abi=hard  -mfpu=fpv4-sp-d16  -mthumb  -MMD  -MP  -Wall  -fno-common  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin  -mapcs  -std=gnu99
LDFLAGS+=--specs=nano.specs --specs=nosys.specs -Xlinker --gc-sections  -Xlinker -static  -Xlinker -z  -Xlinker muldefs  -Xlinker --defsym=__stack_size__=0x400  -Xlinker --defsym=__heap_size__=0x4000

SRC= \
src/main.c \
src/hardware_init.c \
src/imx7_colibri_m4/board.c \
src/imx7_colibri_m4/clock_freq.c \
src/imx7_colibri_m4/gpio_pins.c \
src/imx7_colibri_m4/pin_mux.c \
src/imx7_colibri_m4/rsc_table.c \
src/platform/devices/MCIMX7D/startup/system_MCIMX7D_M4.c \
src/platform/drivers/src/ccm_analog_imx7d.c \
src/platform/drivers/src/ccm_imx7d.c \
src/platform/drivers/src/gpio_imx.c \
src/platform/drivers/src/lmem.c \
src/platform/drivers/src/mu_imx.c \
src/platform/drivers/src/rdc.c \
src/platform/drivers/src/uart_imx.c \
src/platform/drivers/src/wdog_imx.c \
src/platform/utilities/src/debug_console_imx.c \
src/platform/utilities/src/print_scan.c \
rpmsg-lite/lib/rpmsg_lite/porting/platform/imx7d_m4/rpmsg_platform.c \
rpmsg-lite/lib/rpmsg_lite/porting/environment/rpmsg_env_bm.c \
rpmsg-lite/lib/rpmsg_lite/rpmsg_lite.c \
rpmsg-lite/lib/rpmsg_lite/rpmsg_queue.c \
rpmsg-lite/lib/rpmsg_lite/rpmsg_ns.c \
rpmsg-lite/lib/virtio/virtqueue.c \
rpmsg-lite/lib/common/llist.c \
rpmsg-lite/lib/rpmsg_lite/rpmsg_queue.c

OBJS=$(SRC:%.c=%.o)

ASM= \
src/platform/devices/MCIMX7D/startup/gcc/startup_MCIMX7D_M4.S

ASM_OBJS=$(ASM:%.S=%.o)

INCLUDE= \
-Isrc/ \
-Isrc/imx7_colibri_m4 \
-Isrc/platform/devices \
-Isrc/platform/devices/MCIMX7D/startup \
-Isrc/platform/devices/MCIMX7D/include/ \
-Isrc/platform/drivers/inc \
-Isrc/platform/utilities/inc \
-Isrc/platform/utilities/src \
-Icmsis/CMSIS/Core/Include/ \
-Irpmsg-lite/lib/include/ \
-Irpmsg-lite/lib/include/platform/imx7d_m4/

.PHONY: all

all: ocram

.PHONY: ocram tcm

ocram: LDFLAGS+=-Tsrc/platform/devices/MCIMX7D/linker/gcc/MCIMX7D_M4_ocram.ld
ocram: imx7

tcm: imx7
tcm: LDFLAGS+=-Tsrc/platform/devices/MCIMX7D/linker/gcc/MCIMX7D_M4_tcm.ld
tcm: imx7

.PHONY: imx7

imx7: huginn.elf huginn.bin
imx7: CFLAGS+= -DCPU_MCIMX7D_M4

huginn.bin: huginn.elf
	$(OBJCOPY) -Obinary $< $@

huginn.elf: $(OBJS) $(ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(ASM_OBJS) $(LDFLAGS)
	$(STRIP) huginn.elf

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)
