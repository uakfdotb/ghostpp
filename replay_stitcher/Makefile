SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
DFLAGS =
OFLAGS = -O3
LFLAGS = -lz
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
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I. -I../ghost/

GHOSTOBJS = crc32.o gameslot.o packed.o replay.o savegame.o util.o
OBJS = replay_stitcher.o
PROGS = ./replay_stitcher

all: $(GHOSTOBJS) $(OBJS) $(PROGS)

./replay_stitcher: $(GHOSTOBJS) $(OBJS) $(COBJS)
	$(C++) -o ./replay_stitcher $(GHOSTOBJS) $(OBJS) $(LFLAGS)

clean:
	rm -f $(GHOSTOBJS) $(OBJS) $(PROGS)

$(GHOSTOBJS): %.o: ../ghost/%.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

./replay_stitcher: $(GHOSTOBJS) $(OBJS)

all: $(PROGS)

crc32.o: ghost.h ../ghost/crc32.h
gameslot.o: ghost.h ../ghost/gameslot.h
packed.o: ghost.h ../ghost/util.h ../ghost/crc32.h ../ghost/packed.h
replay.o: ghost.h ../ghost/util.h ../ghost/packed.h ../ghost/replay.h ../ghost/gameprotocol.h
savegame.o: ghost.h ../ghost/util.h ../ghost/packed.h ../ghost/savegame.h
util.o: ghost.h ../ghost/util.h
replay_stitcher.o: ../ghost/includes.h ../ghost/util.h ../ghost/packed.h ../ghost/replay.h ../ghost/savegame.h
