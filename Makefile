TARGET			= x86_64
DEBUG			= on
PROGRAM			= iocd
GENERIC_OBJS	= iocd.o util.o id.o
INTERFACE_OBJS	= interfaces.o interface.o interface_usbraw.o
DEVICE_OBJS		= devices.o device.o device_atmel.o device_k8055.o
CONTROL_OBJS	= controls.o control.o
HTTP_OBJS		= http_server.o http_page.o
OBJS			= $(GENERIC_OBJS) $(INTERFACE_OBJS) $(DEVICE_OBJS) $(CONTROL_OBJS) $(HTTP_OBJS)
DESTDIR			= /usr/local

ifeq ($(TARGET), x86_64)
	CPPFLAGS	= `pkg-config --cflags libusb-1.0` `pkg-config --cflags libmicrohttpd`
	LDFLAGS		= `pkg-config --libs libusb-1.0` `pkg-config --libs libmicrohttpd` -lrt
endif

ifeq ($(TARGET), i386)
	CPPFLAGS	= `pkg-config --cflags libusb-1.0` `pkg-config --cflags libmicrohttpd`
	LDFLAGS		= `pkg-config --libs libusb-1.0` `pkg-config --libs libmicrohttpd` -lrt
endif

ifeq ($(TARGET), mipsel21)
	LDLIBS 		+= -Wl,-Bstatic -lmicrohttpd -lusb-1.0 -lrt -Wl,-Bdynamic -lpthread
	CPPFLAGS	+= -I./hostinclude
endif

ifeq ($(TARGET), mipsel)
	LDLIBS 		+= -Wl,-Bstatic -lmicrohttpd -lusb-1.0 -lrt -Wl,-Bdynamic -lpthread
	CPPFLAGS	+= -I./hostinclude
endif

include common.mak
