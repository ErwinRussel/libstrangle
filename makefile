CC=gcc
INC=-Isrc
CFLAGS=-rdynamic -fPIC -shared -Wall -std=c99 -fvisibility=hidden $(INC)
LDFLAGS=-Wl,-z,relro,-z,now
LDLIBS=-ldl -lrt -lc

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
libdir=$(exec_prefix)/lib
datarootdir=$(prefix)/share
LIB32_PATH=$(libdir)/libstrangle/lib32
LIB64_PATH=$(libdir)/libstrangle/lib64

SOURCEDIR=src
BUILDDIR=build
COMMON_SOURCES=$(wildcard $(SOURCEDIR)/*.c)
GL_SOURCES=$(COMMON_SOURCES) $(wildcard $(SOURCEDIR)/opengl/*.c)

CXX=g++
CXXFLAGS= -pthread -rdynamic -fPIC -shared -Wall -std=gnu++17 -fvisibility=hidden -Iinclude $(INC) -DVK_USE_PLATFORM_XLIB_KHR -DHAVE_PTHREAD -DHAVE_TIMESPEC_GET
LDXXFLAGS=
LDXXLIBS=-lrt
VK_SOURCES=\
	$(COMMON_SOURCES) \
	$(wildcard $(SOURCEDIR)/vulkan/*.c) \
	$(wildcard $(SOURCEDIR)/vulkan/*.cpp)

.PHONY: all 32-bit 64-bit native ld clean uninstall install install-32 install-64 install-ld install-common install-native

all: 32-bit 64-bit ld

32-bit: \
	$(BUILDDIR)/libstrangle32.so \
	$(BUILDDIR)/libstrangle32_nodlsym.so \
	$(BUILDDIR)/libstrangle_vk32.so

64-bit: \
	$(BUILDDIR)/libstrangle64.so \
	$(BUILDDIR)/libstrangle64_nodlsym.so \
	$(BUILDDIR)/libstrangle_vk64.so

native: \
	$(BUILDDIR)/libstrangle_native.so \
	$(BUILDDIR)/libstrangle_native_nodlsym.so \
	$(BUILDDIR)/libstrangle_vk_native.so

ld: $(BUILDDIR)/libstrangle.conf

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/libstrangle.conf: | $(BUILDDIR)
	@echo "$(LIB32_PATH)/" > $(BUILDDIR)/libstrangle.conf
	@echo "$(LIB64_PATH)/" >> $(BUILDDIR)/libstrangle.conf

$(BUILDDIR)/libstrangle64.so: | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -m64 -o $@ $(GL_SOURCES) $(LDLIBS) -DHOOK_DLSYM

$(BUILDDIR)/libstrangle32.so: | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -m32 -o $@ $(GL_SOURCES) $(LDLIBS) -DHOOK_DLSYM

$(BUILDDIR)/libstrangle64_nodlsym.so: | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -m64 -o $@ $(GL_SOURCES) $(LDLIBS)

$(BUILDDIR)/libstrangle32_nodlsym.so: | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -m32 -o $@ $(GL_SOURCES) $(LDLIBS)

$(BUILDDIR)/libstrangle_vk64.so: | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(LDXXFLAGS) -m64 -o $@ $(VK_SOURCES) $(LDXXLIBS)

$(BUILDDIR)/libstrangle_vk32.so: | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(LDXXFLAGS) -m32 -o $@ $(VK_SOURCES) $(LDXXLIBS)

$(BUILDDIR)/libstrangle_native.so: | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(GL_SOURCES) $(LDLIBS) -DHOOK_DLSYM

$(BUILDDIR)/libstrangle_native_nodlsym.so: | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(GL_SOURCES) $(LDLIBS)

$(BUILDDIR)/libstrangle_vk_native.so: | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(LDXXFLAGS) -o $@ $(VK_SOURCES) $(LDXXLIBS)

install-common:
	install -m 0755 -D -T $(SOURCEDIR)/strangle.sh $(DESTDIR)$(bindir)/strangle
	install -m 0755 -D -T $(SOURCEDIR)/stranglevk.sh $(DESTDIR)$(bindir)/stranglevk
	install -m 0644 -D -T $(SOURCEDIR)/vulkan/libstrangle_vk.json $(DESTDIR)$(datarootdir)/vulkan/implicit_layer.d/libstrangle_vk.json

install-ld: ld
	install -m 0644 -D -T $(BUILDDIR)/libstrangle.conf $(DESTDIR)/etc/ld.so.conf.d/libstrangle.conf
	if [ -z "$(DESTDIR)" ]; then ldconfig; fi

install-32: 32-bit
	install -m 0755 -D -T $(BUILDDIR)/libstrangle32.so $(DESTDIR)$(LIB32_PATH)/libstrangle.so
	install -m 0755 -D -T $(BUILDDIR)/libstrangle32_nodlsym.so $(DESTDIR)$(LIB32_PATH)/libstrangle_nodlsym.so
	install -m 0755 -D -T $(BUILDDIR)/libstrangle_vk32.so $(DESTDIR)$(LIB32_PATH)/libstrangle_vk.so

install-64: 64-bit
	install -m 0755 -D -T $(BUILDDIR)/libstrangle64.so $(DESTDIR)$(LIB64_PATH)/libstrangle.so
	install -m 0755 -D -T $(BUILDDIR)/libstrangle64_nodlsym.so $(DESTDIR)$(LIB64_PATH)/libstrangle_nodlsym.so
	install -m 0755 -D -T $(BUILDDIR)/libstrangle_vk64.so $(DESTDIR)$(LIB64_PATH)/libstrangle_vk.so

install-native: native
	install -m 0755 -D -T $(BUILDDIR)/libstrangle_native.so $(DESTDIR)$(libdir)/libstrangle.so
	install -m 0755 -D -T $(BUILDDIR)/libstrangle_native_nodlsym.so $(DESTDIR)$(libdir)/libstrangle_nodlsym.so
	install -m 0755 -D -T $(BUILDDIR)/libstrangle_vk_native.so $(DESTDIR)$(libdir)/libstrangle_vk.so

install: \
	all \
	install-common \
	install-32 \
	install-64 \
	install-ld

clean:
	rm -f $(BUILDDIR)/libstrangle64.so
	rm -f $(BUILDDIR)/libstrangle32.so
	rm -f $(BUILDDIR)/libstrangle_native.so
	rm -f $(BUILDDIR)/libstrangle64_nodlsym.so
	rm -f $(BUILDDIR)/libstrangle32_nodlsym.so
	rm -f $(BUILDDIR)/libstrangle_native_nodlsym.so
	rm -f $(BUILDDIR)/libstrangle_vk64.so
	rm -f $(BUILDDIR)/libstrangle_vk32.so
	rm -f $(BUILDDIR)/libstrangle_vk_native.so
	rm -f $(BUILDDIR)/libstrangle.conf

uninstall:
	rm -f $(DESTDIR)/etc/ld.so.conf.d/libstrangle.conf
	rm -f $(DESTDIR)$(LIB32_PATH)/libstrangle.so
	rm -f $(DESTDIR)$(LIB64_PATH)/libstrangle.so
	rm -f $(DESTDIR)$(LIB32_PATH)/libstrangle_nodlsym.so
	rm -f $(DESTDIR)$(LIB64_PATH)/libstrangle_nodlsym.so
	rm -f $(DESTDIR)$(LIB32_PATH)/libstrangle_vk.so
	rm -f $(DESTDIR)$(LIB64_PATH)/libstrangle_vk.so
	rm -f $(DESTDIR)$(datarootdir)/vulkan/implicit_layer.d/libstrangle_vk.json
	rm -f $(DESTDIR)$(bindir)/strangle
	rm -f $(DESTDIR)$(bindir)/stranglevk
