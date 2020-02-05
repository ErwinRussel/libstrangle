CC=gcc
INC=-Isrc
CFLAGS=-rdynamic -fPIC -shared -Wall -std=c99 -fvisibility=hidden $(INC)
LDFLAGS=-Wl,-z,relro,-z,now
LDLIBS=-ldl -lrt
prefix=/usr/local
bindir=$(prefix)/bin
libdir=$(prefix)/lib
DOC_PATH=$(prefix)/share/doc/libstrangle
LIB32_PATH=$(libdir)/libstrangle/lib32
LIB64_PATH=$(libdir)/libstrangle/lib64
SOURCEDIR=src/
BUILDDIR=build/
COMMON_SOURCES=$(wildcard $(SOURCEDIR)*.c)
GL_SOURCES=$(COMMON_SOURCES) $(wildcard $(SOURCEDIR)opengl/*.c)

all: \
	$(BUILDDIR)libstrangle64.so \
	$(BUILDDIR)libstrangle32.so \
	$(BUILDDIR)libstrangle64_nodlsym.so \
	$(BUILDDIR)libstrangle32_nodlsym.so \
	$(BUILDDIR)libstrangle.conf

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)libstrangle.conf: $(BUILDDIR)
	@echo "$(LIB32_PATH)/" > $(BUILDDIR)libstrangle.conf
	@echo "$(LIB64_PATH)/" >> $(BUILDDIR)libstrangle.conf

$(BUILDDIR)libstrangle64.so: $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -m64 -o $(BUILDDIR)libstrangle64.so $(GL_SOURCES) $(LDLIBS) -DHOOK_DLSYM

$(BUILDDIR)libstrangle32.so: $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -m32 -o $(BUILDDIR)libstrangle32.so $(GL_SOURCES) $(LDLIBS) -DHOOK_DLSYM

$(BUILDDIR)libstrangle64_nodlsym.so: $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -m64 -o $(BUILDDIR)libstrangle64_nodlsym.so $(GL_SOURCES) $(LDLIBS)

$(BUILDDIR)libstrangle32_nodlsym.so: $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -m32 -o $(BUILDDIR)libstrangle32_nodlsym.so $(GL_SOURCES) $(LDLIBS)

install: all
	install -m 0644 -D -T $(BUILDDIR)libstrangle.conf $(DESTDIR)/etc/ld.so.conf.d/libstrangle.conf
	install -m 0755 -D -T $(BUILDDIR)libstrangle32.so $(DESTDIR)$(LIB32_PATH)/libstrangle.so
	install -m 0755 -D -T $(BUILDDIR)libstrangle64.so $(DESTDIR)$(LIB64_PATH)/libstrangle.so
	install -m 0755 -D -T $(BUILDDIR)libstrangle32_nodlsym.so $(DESTDIR)$(LIB32_PATH)/libstrangle_nodlsym.so
	install -m 0755 -D -T $(BUILDDIR)libstrangle64_nodlsym.so $(DESTDIR)$(LIB64_PATH)/libstrangle_nodlsym.so
	install -m 0755 -D -T $(SOURCEDIR)strangle.sh $(DESTDIR)$(bindir)/strangle
	install -m 0644 -D -T COPYING $(DESTDIR)$(DOC_PATH)/LICENSE
	ldconfig

clean:
	rm -f $(BUILDDIR)libstrangle64.so
	rm -f $(BUILDDIR)libstrangle32.so
	rm -f $(BUILDDIR)libstrangle64_nodlsym.so
	rm -f $(BUILDDIR)libstrangle32_nodlsym.so
	rm -f $(BUILDDIR)libstrangle.conf

uninstall:
	rm -f $(DESTDIR)/etc/ld.so.conf.d/libstrangle.conf
	rm -f $(DESTDIR)$(LIB32_PATH)/libstrangle.so
	rm -f $(DESTDIR)$(LIB64_PATH)/libstrangle.so
	rm -f $(DESTDIR)$(LIB32_PATH)/libstrangle_nodlsym.so
	rm -f $(DESTDIR)$(LIB64_PATH)/libstrangle_nodlsym.so
	rm -f $(DESTDIR)$(bindir)/strangle
	rm -f $(DESTDIR)$(DOC_PATH)/LICENSE
