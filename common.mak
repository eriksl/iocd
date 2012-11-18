WARNINGS		= -Wall -Wextra -Wshadow -Wundef -Wformat=2 -Winit-self -Wunused -Werror -Wpointer-arith -Wcast-qual -Wmultichar -fno-rtti

ifneq ($(DEBUG), on)
CPPFLAGS		+= -O2 $(WARNINGS)
LDFLAGS			+= -s
else
CPPFLAGS		+= -O0 -g $(WARNINGS)
LDFLAGS			+= -g
endif

ifeq ($(TARGET), x86_64)
	CC			= gcc
	CPP			= g++
	CPPFLAGS	+= -DTARGET=x86_64 -DTARGET_X86_64=1
endif

ifeq ($(TARGET), i386)
	CC			= gcc
	CPP			= g++
	CPPFLAGS	+= -m32 -DTARGET=i386 -DTARGET_I386=1
	LDFLAGS		+= -m32
endif

ifeq ($(TARGET), mipsel)
	CC			= mipsel-oe-linux-gcc
	CPP			= mipsel-oe-linux-g++
	CPPFLAGS	+= -I/home/erik/src/libmicrohttpd/mips32el/usr/include
	CPPFLAGS	+= -I/home/erik/src/openpli/build-vuultimo/tmp/sysroots/vuultimo/usr/include/libusb-1.0
	CPPFLAGS	+= -DTARGET=mipsel -DTARGET_MIPSEL=1
	LDFLAGS		+= -L/home/erik/src/libmicrohttpd/mips32el/usr/lib
	LDFLAGS		+= -L/home/erik/src/openpli/build-vuultimo/tmp/sysroots/vuultimo/lib
endif

.PHONY:			all depend clean pristine install rpm dpkg

DATE			=	`date '+%Y%m%d%H%M'`
VERSION			=	daily-$(DATE)

RPM				=	`pwd`/rpm
RPMTARBALL		=	$(RPM)/$(PROGRAM).tar
RPMPKGDIR		=	$(RPM)/$(TARGET)
RPMDEBUGDIR		=	$(RPMPKGDIR)/debug
RPMBUILDDIR		=	$(RPM)/build
RPMTMP			=	$(RPM)/tmp
RPMSPECIN		=	$(RPM)/$(PROGRAM).spec
RPMSPECOUTDIR	=	$(RPMBUILDDIR)/rpm
RPMSPECOUT		=	$(RPMSPECOUTDIR)/$(PROGRAM).spec

DPKG			=	`pwd`/dpkg
DPKGTARBALL 	=	$(DPKG)/$(PROGRAM).tar
DPKGDEBIANDIR	=	$(DPKG)/DEBIAN
DPKGCONFIGURE	=	$(DPKG)/configure.dpkg
DPKGPKGDIR		=	$(DPKG)/$(TARGET)
DPKGBUILDDIR	=	$(DPKG)/build
DPKGDESTDIR		=	$(DPKGBUILDDIR)/dpkg/root
DPKGDEBIANDESTD	=	$(DPKGDESTDIR)/DEBIAN
DPKGCHANGELOG	=	$(DPKGDEBIANDESTD)/changelog
DPKGCONTROL		=	$(DPKGDEBIANDESTD)/control

DEPS			=	$(patsubst %.o,.%.d, $(OBJS))

all:		depend $(PROGRAM)

ifeq ($(DEPS), "")
else
-include .deps
endif

depend:		.deps

.deps:		$(DEPS)
			@cat $^ /dev/null > $@

.%.d:		%.cpp
			@$(CPP) $(CPPFLAGS) -M $^ -o $@

.%.d:		%.c
			@$(CPP) $(CPPFLAGS) -M $^ -o $@

%.o:		%.cpp
			@echo "CPP $< -> $@"
			@$(CPP) $(CPPFLAGS) -c $< -o $@

$(PROGRAM):	$(OBJS)
			@echo "LD $@"
			@$(CPP) $(LDFLAGS) $^ $(LDLIBS) -o $@ 

install:	$(PROGRAM)
			@echo "INSTALL $(PROGRAM) -> $(DESTDIR)/bin"
			@mkdir -p $(DESTDIR)/bin
			@cp $(PROGRAM) $(DESTDIR)/bin
			@-chown root:root $(DESTDIR)/bin/$(PROGRAM)
			@-chmod 755 $(DESTDIR)/bin/$(PROGRAM)

clean:
			@echo "CLEAN"
			rm -f $(PROGRAM) $(OBJS) 2> /dev/null || true

pristine:	
			@echo "PRISTINE"
			git clean -f -d -q

rpm:
#
			@echo "PREPARE $(VERSION)"
			@-rm -rf $(RPMPKGDIR) $(RPMBUILDDIR) $(RPMTMP) 2> /dev/null || true
			@mkdir -p $(RPMPKGDIR) $(RPMBUILDDIR) $(RPMTMP) $(RPMSPECOUTDIR)
			@sed $(RPMSPECIN) -e "s/%define dateversion.*/%define dateversion $(DATE)/" > $(RPMSPECOUT)
#
			@echo "TAR $(RPMTARBALL)"
			@-rm -f $(RPMTARBALL) 2> /dev/null
#			tar cf $(RPMTARBALL) -C .. --exclude rpm --exclude .git $(PROGRAM)
			tar cf $(RPMTARBALL) -C .. --exclude rpm $(PROGRAM)
#
			@echo "CREATE RPM $(RPMSPECOUT)"
			@rpmbuild -bb $(RPMSPECOUT)
#
			@-rm -rf $(RPMDEBUGDIR) 2> /dev/null || true
			@-mkdir -p $(RPMDEBUGDIR) 2> /dev/null || true
			@-mv rpm/$(TARGET)/$(PROGRAM)-debuginfo-*.rpm rpm/$(TARGET)/debug 2> /dev/null || true
			@-rm -rf $(RPMBUILDDIR) $(RPMTMP)

dpkg:
#
			@echo "PREPARE $(VERSION)"
			@-rm -f $(DPKGPKGDIR) $(DPKGBUILDDIR) $(DPKGDESTDIR) 2> /dev/null || true
			@mkdir -p $(DPKGPKGDIR) $(DPKGBUILDDIR)
#
			@echo "TAR $(DPKGTARBALL)"
			@-rm -f $(DPKGTARBALL) 2> /dev/null
			@git archive -o $(DPKGTARBALL) HEAD
			@tar xf $(DPKGTARBALL) -C $(DPKGBUILDDIR)
#
			@echo "DUP $(DPKGDEBIANDIR)"
			@mkdir -p $(DPKGDEBIANDESTD)
			@cp $(DPKGDEBIANDIR)/* $(DPKGDEBIANDESTD)
#
			@echo "CHANGELOG $(DPKGCHANGELOG)"
			@-rm -f $(DPKGCHANGELOG) 2> /dev/null
			@echo "$(PROGRAM) ($(VERSION)) stable; urgency=low" > $(DPKGCHANGELOG)
			@echo >> $(DPKGCHANGELOG)
			@git log | head -100 >> $(DPKGCHANGELOG)
			@sed --in-place $(DPKGCONTROL) -e "s/^Version:.*/Version: `date "+%Y%m%d"`/"
			@sed --in-place $(DPKGCONTROL) -e "s/^Architecture:.*/Architecture: $(TARGET)/"
#
			@echo "CONFIGURE $(DPKGCONFIGURE)"
			@(cd $(DPKGBUILDDIR); $(DPKGCONFIGURE))
#
			@echo "BUILD $(DPKGBUILDDIR)"
			@$(MAKE) -C $(DPKGBUILDDIR) all
#
			@echo "CREATE DEB"
			@fakeroot /bin/sh -c "\
				$(MAKE) -C $(DPKGBUILDDIR) DESTDIR=$(DPKGDESTDIR) install; \
				dpkg --build $(DPKGDESTDIR) $(DPKGPKGDIR)"

rpminstall:
			@echo "INSTALL"
			@sudo rpm -Uvh --force rpm/$(TARGET)/*.rpm
