TARGET			= x86_64
DEBUG			= off
PROGRAM			= iocd
GENERIC_OBJS	= iocd.o syslog.o identity.o
INTERFACE_OBJS	= interfaces.o interface.o interface_elv.o
DEVICE_OBJS		= devices.o device.o device_atmel.o device_tmp275.o
CONTROL_OBJS	= controls.o control.o control_atmel.o control_tmp275.o
HTTP_OBJS		= http_server.o http_page.o
OBJS			= $(GENERIC_OBJS) $(INTERFACE_OBJS) $(DEVICE_OBJS) $(CONTROL_OBJS) $(HTTP_OBJS)

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
