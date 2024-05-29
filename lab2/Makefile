CC = gcc
CFLAGS = -Wall -g -std=gnu99
LD = gcc
LDFLAGS = -g 
VPATH := source
LDLIBS = 

SRCS   = paster.c
OBJS   = paster.o
TARGETS= paster

all: ${TARGETS}

paster.out: $(OBJS) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

%.d: %.c
	gcc -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *~ *.d *.o $(TARGETS) 