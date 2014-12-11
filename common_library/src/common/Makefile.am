include $(top_srcdir)/Make.rules
AUTOMAKE_OPTIONS= foreign

lib_LIBRARIES = libmcommon.a

libmcommon_OBJECTS=../util/*.o ../sys/*.o ../net/*.o
AM_LDFLAGS  += -fPIC -shared 
AM_CXXFLAGS += -fPIC

all: $(lib_LIBRARIES) libmcommon.so

libmcommon.a:
	$(AR) $(ARFLAGS) $@ $(libmcommon_OBJECTS)
	ranlib $@

libmcommon.so:
	 $(CXX) -o $@ $(libmcommon_OBJECTS) $(AM_LDFLAGS) -pthread

clean:
	rm -f $(lib_LIBRARIES) libmcommon.so

install:
	cp $(lib_LIBRARIES) $(prefix)/lib
	cp libmcommon.so $(prefix)/lib

uninstall:
	rm -f $(prefix)/lib/$(lib_LIBRARIES)
	rm -f $(prefix)/lib/libmcommon.so
