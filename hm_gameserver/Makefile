CFLAGS=-Wall -DMOD_$(mod) -g -Isrc/include -Isrc/include/proto -I../hm_base/src/include -I/usr/include/json-c/ -Iscript/mechanics/include #-DTURNOFF_TIMER -DMANA_ZERO -DTEST_DECK -DSPELL_ZERO_MANA
LIBS=-lev -lhmbase -lm -ljson-c -lcouchbase
LIBPATH=-L../hm_base/lib
TARGET=hm_gameserver

SOURCES=$(shell find src -type f -iname '*.c')

OBJECTS=$(foreach x, $(basename $(SOURCES)), $(x).o)

all: base script hm_gameserver

base:
	$(MAKE) -C ../hm_base target=game

hm_gameserver:
	$(CC) $(SOURCES) $(CFLAGS) $(LIBPATH) -o $(TARGET) $(LIBS)

script:
	mkdir -p script/mechanics/include
	cd script/ && python entities.py
	cd script/mechanics/ && python main.py
	cd script/ && python flags.py
	cd script/ && python entities.py
	cd script/mechanics/ && python main.py

clean:
	rm -f $(OBJECTS) $(TARGET) tags \
		script/flags_sections \
		script/mechanics/*.pyc \
		src/flag_def.c \
		src/proto/ent.c \
		src/proto/ent_sections.c \
		src/levelup.c \
		src/include/levelup.h \
		src/include/flag_def.h \
		src/include/proto/ent_gen.h \
		script/mechanics/include/*.h

.PHONY: all hm_gameserver script
