INCS=-Isrc/include -I../hm_base/src/include -I/usr/include/json-c/
LIB_PATH=-L../hm_base/lib
LIB=-lhmbase -lm -lev -ljson-c -lcouchbase

all: base mods
	$(CC) -g -DMOD_$(mod) $(INCS) $(LIB_PATH) src/*.c $(LIB) -o hm_lobbyserver

base:
	$(MAKE) -C ../hm_base target=lobby

mods:
	cd script/ && python main.py

clean:
	rm hm_lobbyserver
