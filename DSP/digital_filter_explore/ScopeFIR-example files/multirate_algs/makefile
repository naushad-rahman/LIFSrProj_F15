CC=gcc
LD=gcc
INCLUDES= -I.

DFLAGS=
CFLAGS=  -Wall -g $(INCLUDES) $(DFLAGS)
LIBRARIES = -lm
TARGET=mr_test

OBJECTS=  \
	decim.o \
	interp.o \
	mr_test.o \
	resamp.o

###################################################################
# Build targets 
###################################################################
all : $(TARGET)

$(TARGET): $(OBJECTS) 
	$(LD) -o $(TARGET) $(OBJECTS) $(LIBRARIES)

$(OBJECTS) : %.o : %.c
	$(CC) -c $(CFLAGS) $<  -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)
