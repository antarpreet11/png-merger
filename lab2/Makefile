CC = gcc
CFLAGS = -Wall -g -std=gnu99
LD = gcc
LDFLAGS = -g 
VPATH := source
LDLIBS = -lcurl -pthread

SRCS   = paster.c main_write_header_cb.c
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

.PHONY: imgclean 
imgclean:
	rm -rf source/img/*.png