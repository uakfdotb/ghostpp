SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS = -DGHOST_MYSQL
OFLAGS = -O3
LFLAGS = -L. -L../bncsutil/src/bncsutil/ -L../StormLib/stormlib/ -lbncsutil -lpthread -ldl -lz -lStorm -lmysqlclient_r -lboost_date_time-mt -lboost_thread-mt -lboost_system-mt -lboost_filesystem-mt
CFLAGS =

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
else
LFLAGS += -lrt
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I. -I../bncsutil/src/ -I../StormLib/

ifeq ($(SYSTEM),Darwin)
CFLAGS += -I../mysql/include/
endif

OBJS = bncsutilinterface.o bnet.o bnetprotocol.o bnlsclient.o bnlsprotocol.o commandpacket.o config.o crc32.o csvparser.o game.o game_admin.o game_base.o gameplayer.o gameprotocol.o gameslot.o ghost.o ghostdb.o ghostdbmysql.o ghostdbsqlite.o gpsprotocol.o language.o map.o packed.o replay.o savegame.o sha1.o socket.o stats.o statsdota.o statsw3mmd.o util.o
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

bncsutilinterface.o: ghost.h includes.h util.h bncsutilinterface.h
bnet.o: ghost.h includes.h util.h config.h language.h socket.h commandpacket.h ghostdb.h bncsutilinterface.h bnlsclient.h bnetprotocol.h bnet.h map.h packed.h savegame.h replay.h gameprotocol.h game_base.h
bnetprotocol.o: ghost.h includes.h util.h bnetprotocol.h
bnlsclient.o: ghost.h includes.h util.h socket.h commandpacket.h bnlsprotocol.h bnlsclient.h
bnlsprotocol.o: ghost.h includes.h util.h bnlsprotocol.h
commandpacket.o: ghost.h includes.h commandpacket.h
config.o: ghost.h includes.h config.h
crc32.o: ghost.h includes.h crc32.h
csvparser.o: csvparser.h
game.o: ghost.h includes.h util.h config.h language.h socket.h ghostdb.h bnet.h map.h packed.h savegame.h gameplayer.h gameprotocol.h game_base.h game.h stats.h statsdota.h statsw3mmd.h
game_admin.o: ghost.h includes.h util.h config.h language.h socket.h ghostdb.h bnet.h map.h packed.h savegame.h replay.h gameplayer.h gameprotocol.h game_base.h game_admin.h
game_base.o: ghost.h includes.h util.h config.h language.h socket.h ghostdb.h bnet.h map.h packed.h savegame.h replay.h gameplayer.h gameprotocol.h game_base.h next_combination.h
gameplayer.o: ghost.h includes.h util.h language.h socket.h commandpacket.h bnet.h map.h gameplayer.h gameprotocol.h gpsprotocol.h game_base.h
gameprotocol.o: ghost.h includes.h util.h crc32.h gameplayer.h gameprotocol.h game_base.h
gameslot.o: ghost.h includes.h gameslot.h
ghost.o: ghost.h includes.h util.h crc32.h sha1.h csvparser.h config.h language.h socket.h ghostdb.h ghostdbsqlite.h ghostdbmysql.h bnet.h map.h packed.h savegame.h gameplayer.h gameprotocol.h gpsprotocol.h game_base.h game.h game_admin.h
ghostdb.o: ghost.h includes.h util.h config.h ghostdb.h
ghostdbmysql.o: ghost.h includes.h util.h config.h ghostdb.h ghostdbmysql.h
ghostdbsqlite.o: ghost.h includes.h util.h config.h ghostdb.h ghostdbsqlite.h
gpsprotocol.o: ghost.h util.h gpsprotocol.h
language.o: ghost.h includes.h config.h language.h
map.o: ghost.h includes.h util.h crc32.h sha1.h config.h map.h
packed.o: ghost.h includes.h util.h crc32.h packed.h
replay.o: ghost.h includes.h util.h packed.h replay.h gameprotocol.h
savegame.o: ghost.h includes.h util.h packed.h savegame.h
sha1.o: sha1.h
socket.o: ghost.h includes.h util.h socket.h
stats.o: ghost.h includes.h stats.h
statsdota.o: ghost.h includes.h util.h ghostdb.h gameplayer.h gameprotocol.h game_base.h stats.h statsdota.h
statsw3mmd.o: ghost.h includes.h util.h ghostdb.h gameprotocol.h game_base.h stats.h statsw3mmd.h
util.o: ghost.h includes.h util.h
