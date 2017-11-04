# Run "make help" for target help.

MCU          = atmega32u4
ARCH         = AVR8
F_CPU        = 2000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = sensor
SRC          = $(TARGET).c usb.c adc.c spi.c i2c.c bme280.c \
ioLibrary_Driver/Ethernet/socket.c \
ioLibrary_Driver/Ethernet/wizchip_conf.c \
ioLibrary_Driver/Ethernet/W5500/w5500.c \
ioLibrary_Driver/Internet/DHCP/dhcp.c
CC_FLAGS     = -I ioLibrary_Driver/Ethernet/ \
-I ioLibrary_Driver/Ethernet/W5500/ \
-I ioLibrary_Driver/Internet/DHCP/
# floating point support in fprintf()
LD_FLAGS     = -Wl,-u,vfprintf -lprintf_flt
AVRDUDE_PROGRAMMER = atmelice

# Default target
all:

# Include common DMBS build system modules
DMBS_PATH ?= dmbs/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/doxygen.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk
