include $(top_srcdir)/Make.rules
AUTOMAKE_OPTIONS= foreign

mooon_OBJECTS=../utils/*.o ../sys/*.o ../net/*.o
AM_LDFLAGS  += -fPIC -shared 
AM_CXXFLAGS += -fPIC

all: libmooon.a libmooon.so

libmooon.a: $(mooon_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^
	ranlib $@

libmooon.so: $(mooon_OBJECTS)
	 $(CXX) -o $@ $^ $(AM_LDFLAGS) -pthread

clean:
	rm -f libmooon.a libmooon.so

install:
	cp libmooon.a $(prefix)/lib
	cp libmooon.so $(prefix)/lib

uninstall:
	rm -f $(prefix)/lib/libmooon.a
	rm -f $(prefix)/lib/libmooon.so
