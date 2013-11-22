
CFLAGS = -g

MAPGEN_OBJECTS = mapgen.o rnd.o z.o
ISLAND_OBJECTS = island.o rnd.o

all:	mapgen island

mapgen:	$(MAPGEN_OBJECTS)
	$(CC) $(CFLAGS) -o mapgen $(MAPGEN_OBJECTS) $(LFLAGS)

island:	$(ISLAND_OBJECTS)
	$(CC) $(CFLAGS) -o island $(ISLAND_OBJECTS) $(LFLAGS)

clean:
	rm -f *.o

clobber:	clean
	rm -f mapgen
	rm -f island

mapgen.o:	mapgen.c z.h
island.o:	island.c
z.o:		z.c z.h
rnd.o:		rnd.c
