TARGET			= x86_64
DEBUG			= off
PROGRAM			= iocd
GENERIC_OBJS	= iocd.o util.o id.o
INTERFACE_OBJS	= interfaces.o interface.o interface_elv.o interface_usbraw.o
DEVICE_OBJS		= devices.o device.o device_atmel.o device_tmp275.o device_digipicco.o device_tsl2550.o device_ds1731.o device_i2c.o device_k8055.o device_usbraw.o
CONTROL_OBJS	= controls.o control.o
HTTP_OBJS		= http_server.o http_page.o
OBJS			= $(GENERIC_OBJS) $(INTERFACE_OBJS) $(DEVICE_OBJS) $(CONTROL_OBJS) $(HTTP_OBJS)
DESTDIR			= /usr/local

ifeq ($(TARGET), x86_64)
	CPPFLAGS	= `pkg-config --cflags libusb-1.0`
	LDFLAGS		= `pkg-config --libs libusb-1.0`
	LDLIBS		+= -Wl,-Bstatic -lmicrohttpd -Wl,-Bdynamic -lrt -lpthread
endif

ifeq ($(TARGET), i386)
	CPPFLAGS	= `pkg-config --cflags libusb-1.0`
	LDFLAGS		= `pkg-config --libs libusb-1.0`
	LDLIBS += -Wl,-Bstatic -lmicrohttpd -lrt -Wl,-Bdynamic -lpthread
endif

ifeq ($(TARGET), mipsel)
	LDLIBS 		+= -Wl,-Bstatic -lmicrohttpd -lusb-1.0 -lrt -Wl,-Bdynamic -lpthread
endif

include common.mak
