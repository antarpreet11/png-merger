CC = gcc
CFLAGS = -Wall -g -std=gnu99
LD = gcc
LDFLAGS = -g 
VPATH := source
LDLIBS = -lcurl -lz -pthread

SRCS   = paster.c main_write_header_cb.c $(wildcard source/lib/*.c)
OBJS   = $(SRCS:.c=.o)
TARGETS= paster

all: $(TARGETS)

paster: $(OBJS)
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.d: %.c
	$(CC) -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *~ *.d *.o $(TARGETS)
	rm -f source/lib/*~ source/lib/*.d source/lib/*.o
	rm -rf source/img/*.png
	rm -rf all.png