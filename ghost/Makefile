SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS =
OFLAGS = -O3
LFLAGS = -L. -L../bncsutil/src/bncsutil/ -L../StormLib/stormlib/ -lbncsutil -lpthread -ldl -lStorm
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

CFLAGS += $(OFLAGS) $(DFLAGS) -I. -I../bncsutil/src/ -I../StormLib/

OBJS = bncsutilinterface.o bnet.o bnetprotocol.o commandpacket.o config.o crc32.o csvparser.o game.o gameplayer.o gameprotocol.o gameslot.o ghost.o ghostdb.o ghostdbsqlite.o language.o map.o socket.o stats.o statsdota.o util.o
COBJS = sqlite3.o
PROGS = ./ghost++

all: $(OBJS) $(COBJS) $(PROGS)

./ghost++: $(OBJS) $(COBJS)
	$(C++) -o ./ghost++ $(OBJS) $(COBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

./ghost++: $(OBJS) $(COBJS)

all: $(PROGS)

bncsutilinterface.o: ghost.h util.h bncsutilinterface.h
bnet.o: ghost.h util.h config.h language.h socket.h commandpacket.h ghostdb.h bncsutilinterface.h bnetprotocol.h bnet.h map.h gameprotocol.h game.h
bnetprotocol.o: ghost.h util.h bnetprotocol.h
commandpacket.o: ghost.h commandpacket.h
config.o: ghost.h util.h config.h
crc32.o: ghost.h crc32.h
csvparser.o: csvparser.h
game.o: ghost.h util.h config.h language.h socket.h ghostdb.h bnet.h map.h gameplayer.h gameprotocol.h game.h stats.h statsdota.h
gameplayer.o: ghost.h util.h language.h socket.h commandpacket.h bnet.h map.h gameplayer.h gameprotocol.h game.h
gameprotocol.o: ghost.h util.h crc32.h gameplayer.h gameprotocol.h game.h
gameslot.o: ghost.h gameslot.h
ghost.o: ghost.h util.h crc32.h csvparser.h config.h language.h socket.h ghostdb.h ghostdbsqlite.h bnet.h map.h gameprotocol.h game.h
ghostdb.o: ghost.h config.h ghostdb.h
ghostdbsqlite.o: ghost.h util.h config.h ghostdb.h ghostdbsqlite.h
language.o: ghost.h config.h language.h
socket.o: ghost.h util.h socket.h
stats.o: ghost.h stats.h
statsdota.o: ghost.h util.h ghostdb.h gameplayer.h gameprotocol.h game.h stats.h statsdota.h
util.o: ghost.h util.h
