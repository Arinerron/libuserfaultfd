.PHONY : clean

CPPFLAGS= -fPIC -g -I$(LIBUSERFAULTFD_INCDIR) -static
LDFLAGS= -shared

INCDIR=/usr/include
LIBDIR=/usr/lib

LIBUSERFAULTFD_NAME=libuserfaultfd.so
LIBUSERFAULTFD_SRCDIR=./src
LIBUSERFAULTFD_INCDIR=./inc
LIBUSERFAULTFD_LIBDIR=./lib

SOURCES = $(shell echo $(LIBUSERFAULTFD_SRCDIR)/*.c)
HEADERS = $(shell echo $(LIBUSERFAULTFD_INCDIR)/*.h)
OBJECTS=$(SOURCES:.c=.o)

TARGET=$(LIBUSERFAULTFD_LIBDIR)/$(LIBUSERFAULTFD_NAME)

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: install
install: $(TARGET)
	mkdir -p $(LIBDIR) $(INCDIR)
	cp $(TARGET) $(LIBDIR)/$(LIBUSERAULTFD_NAME)
	cp $(HEADERS) $(INCDIR)

.PHONY: uninstall
uninstall: $(TARGET)
	rm -f $(LIBDIR)/$(LIBUSERFAULTFD_NAME)

$(TARGET) : $(OBJECTS)
	mkdir -p $(LIBUSERFAULTFD_LIBDIR)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)
