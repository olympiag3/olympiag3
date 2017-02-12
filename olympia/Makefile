CC = gcc
CFLAGS = -g -I. -I../include -m32

GAME_OBJECTS = add.o adv.o alchem.o art.o basic.o beast.o build.o buy.o \
  c1.o c2.o check.o cloud.o code.o combat.o day.o dir.o display.o eat.o \
  faery.o garr.o gate.o glob.o gm.o hades.o immed.o input.o io.o loc.o \
  lore.o main.o make.o move.o necro.o npc.o order.o perm.o produce.o \
  pw.o quest.o relig.o report.o rnd.o savage.o scry.o seed.o sout.o \
  stack.o stealth.o storm.o summary.o swear.o tunnel.o u.o use.o z.o

OBJECTS = $(patsubst %.o,obj/%.o,$(GAME_OBJECTS))

all: olympia

olympia: objdir $(OBJECTS)
	$(CC) $(CFLAGS) -lm -o olympia $(OBJECTS)

clean:
	rm -f $(OBJECTS)
	if [ -d obj ]; then rmdir obj; fi

objdir:
	if [ ! -d obj ]; then mkdir obj; fi

$(patsubst %.o,obj/%.o,$(GAME_OBJECTS)): obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

