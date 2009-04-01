SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
DFLAGS =
OFLAGS = -O3
LFLAGS = -lmysqlclient
CFLAGS =

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I. -I../ghost/

GHOSTOBJS = config.o
OBJS = elo.o update_dota_elo.o
PROGS = ./update_dota_elo

all: $(GHOSTOBJS) $(OBJS) $(PROGS)

./update_dota_elo: $(GHOSTOBJS) $(OBJS) $(COBJS)
	$(C++) -o ./update_dota_elo $(GHOSTOBJS) $(OBJS) $(LFLAGS)

clean:
	rm -f $(GHOSTOBJS) $(OBJS) $(PROGS)

$(GHOSTOBJS): %.o: ../ghost/%.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

./update_dota_elo: $(GHOSTOBJS) $(OBJS)

all: $(PROGS)

config.o: ../ghost/ghost.h ../ghost/config.h
elo.o: elo.h
update_dota_elo.o: ../ghost/config.h elo.h
