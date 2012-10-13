TARGET		= x86_64
DEBUG		= off
PROGRAM		= iocd
OBJS		=  iocd.o  syslog.o  interfaces.o  interface.o  interface_elv.o  devices.o  device.o  device_atmel.o  controls.o  control.o
DEPS		= .iocd.d .syslog.d .interfaces.d .interface.d .interface_elv.d .devices.d .device.d .device_atmel.d .controls.d .control.d
OBJS		+=  control_atmel.o
DEPS		+= .control_atmel.d

ifeq ($(TARGET), x86_64)
	LDLIBS += -Wl,-Bstatic -lmicrohttpd -Wl,-Bdynamic -lboost_regex -lrt -lpthread
endif

ifeq ($(TARGET), i386)
	LDLIBS += -Wl,-Bstatic -lmicrohttpd -lboost_regex -lrt -Wl,-Bdynamic -lpthread
endif

ifeq ($(TARGET), mipsel)
	LDLIBS += -Wl,-Bstatic -lmicrohttpd -lboost_regex -lrt -Wl,-Bdynamic -lpthread
endif

include common.mak
