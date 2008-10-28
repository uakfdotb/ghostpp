########################################################################
#
# Makefile for compiling StormLib under linux
#
# Author: Marko Friedemann <marko.friedemann@bmx-chemnitz.de>
# Created at: Mon Jan 29 18:26:01 CEST 2001
# Computer: whiplash.flachland-chemnitz.de 
# System: Linux 2.4.0 on i686
#    
# Copyright (c) 2001 BMX-Chemnitz.DE All rights reserved.
#
########################################################################

FILES.cpp = SCommon.cpp SFileCreateArchiveEx.cpp SFileFindFile.cpp \
            SFileOpenArchive.cpp SFileOpenFileEx.cpp SFileReadFile.cpp \
            SFileCompress.cpp SFileDecompress.cpp \
            SListFile.cpp \
            wavunp/wavunp1.cpp wavunp/wavunp2.cpp \
            dcl/explode.c \
FILES.o = $(FILES.cpp:.cpp=.o)

include Makefile.config

LIB = libStorm.so

CFLAGS = $(CFLAGS_ADD) -D__SYS_ZLIB
I_FLAGS = $(I_FLAGS_ADD)
LDFLAGS = $(LDFLAGS_ADD) -lz

all: .dep $(LIB)

$(LIB): $(FILES.o)
	ld -shared $(LDFLAGS) -o $(LIB) $(FILES.o)

include .dep

%.o: %.cpp
	$(G++) $(CFLAGS) -c $< -o $@

clean:
	-rm $(FILES.o)

new: clean all

mrproper: clean
	-rm $(LIB) .dep

mrnew: mrproper new

install: $(LIB)
	install $(I_FLAGS) $(LIB) /usr/local/lib
	mkdir -p /usr/local/include/StormLib
	cp Storm.h /usr/local/include/StormLib
	cp StormPort.h /usr/local/include/StormLib
	ldconfig

.dep:
	$(G++) -MM $(CFLAGS) $(FILES.cpp) > .dep
