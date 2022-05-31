ifeq ($(target),game)
CFLAGS+=-g -Wall -DHM_GAMESERVER
else
CFLAGS+=-g -Wall -DHM_LOBBYSERVER
endif
INCS= -Isrc/include
LIBS= -lcouchbase

compile = $(CC) -c $(CFLAGS) $(INCS) $(LIBS) src/$(1).c -o src/$(1).o

game: hmbase

hmbase: src/*.c src/include/*.h
	rm -rf lib/
	$(call compile,log)
	$(call compile,hashtable)
	$(call compile,rb)
	$(call compile,pool)
	$(call compile,connector)
	$(call compile,couchbase)
	$(call compile,async_client)
	mkdir -p lib/
	ar cq lib/libhmbase.a src/*.o

clean:
	rm -rf src/*.o ./lib/

.PHONY: static hmbase
