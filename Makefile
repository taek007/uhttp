CC=gcc
LD=gcc

CCFLAGS= -g
LDFLAGS= -g

OBJS=http.o server.o mem.o

all:uhttp
everything:uhttp

%.o:	%.c
	$(CC) $(CCFLAGS) -c $<

uhttp:$(OBJS) main.o 
	$(LD) $(LDFLAGS) main.o $(OBJS) -o $@

clean:
	rm $(OBJS) main.o
	rm uhttp 
