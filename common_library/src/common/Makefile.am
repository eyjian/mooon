include $(top_srcdir)/Make.rules
AUTOMAKE_OPTIONS= foreign

lib_LIBRARIES = libmooon.a

libmooon_OBJECTS=../utils/*.o ../sys/*.o ../net/*.o
AM_LDFLAGS  += -fPIC -shared 
AM_CXXFLAGS += -fPIC

all: $(lib_LIBRARIES) libmooon.so

libmooon.a:
	$(AR) $(ARFLAGS) $@ $(libmooon_OBJECTS)
	ranlib $@

libmooon.so:
	 $(CXX) -o $@ $(libmooon_OBJECTS) $(AM_LDFLAGS) -pthread

clean:
	rm -f $(lib_LIBRARIES) libmooon.so

install:
	cp $(lib_LIBRARIES) $(prefix)/lib
	cp libmooon.so $(prefix)/lib

uninstall:
	rm -f $(prefix)/lib/$(lib_LIBRARIES)
	rm -f $(prefix)/lib/libmooon.so
