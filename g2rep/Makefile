CFLAGS = -g

G2REP_OBJECTS = rep.o

all:	g2rep

g2rep:	$(G2REP_OBJECTS)
	$(CC) $(CFLAGS) -o g2rep $(G2REP_OBJECTS) $(LFLAGS)

clean:
	rm -f *.o

clobber:	clean
	rm -f g2rep

rep.o:		rep.c
