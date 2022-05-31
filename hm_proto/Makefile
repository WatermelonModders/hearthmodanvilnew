INCS= -Isrc/include -I../hm_base/src/include
LIBS_PATH= -L../hm_base/lib/
LIBS= -lhmbase
CFLAGS+= -g

compile = $(CC) -c $(CFLAGS) $(INCS) $(LIBS_PATH) src/$(1).c -o src/$(1).o $(LIBS)

static: hmproto

hmproto:
	rm -rf lib/
	$(call compile,alloptions)
	$(call compile,bnet)
	$(call compile,chooseentities)
	$(call compile,chooseoption)
	$(call compile,creategame)
	$(call compile,packet)
	$(call compile,entity)
	$(call compile,entitychoices)
	$(call compile,game_entity)
	$(call compile,gamesetup)
	$(call compile,handshake)
	$(call compile,hide)
	$(call compile,meta)
	$(call compile,mouseinfo)
	$(call compile,option)
	$(call compile,platform)
	$(call compile,player)
	$(call compile,powerend)
	$(call compile,powerhistory)
	$(call compile,powerhistorydata)
	$(call compile,powerstart)
	$(call compile,proto)
	$(call compile,suboption)
	$(call compile,tag)
	$(call compile,tagchange)
	$(call compile,turntimer)
	$(call compile,userui)
	mkdir -p lib/
	ar cq lib/libhmproto.a src/*.o

clean:
	rm -rf src/*.o ./lib/

.PHONY: static hmproto
