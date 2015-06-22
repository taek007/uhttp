CC=gcc
LD=gcc

CCFLAGS= -g
LDFLAGS= -g
LIBS=-ljson -lpthread

OBJS=http.o server.o mem.o fastcgi.o conf.o log.o svrctl.o

all:uhttp
everything:uhttp

%.o:	%.c
	$(CC) $(CCFLAGS) -c $<

uhttp:$(OBJS) main.o 
	$(LD) $(LDFLAGS) main.o $(OBJS) -o $@ $(LIBS)

clean:
	rm $(OBJS)
	rm uhttp test main.o test.o

test: test.o $(OBJS)
	$(LD) $(LDFLAGS) test.o $(OBJS) -o $@ $(LIBS)
	./test
