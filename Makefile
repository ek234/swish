BASE = src

CC = gcc
CFLAGS =

IDIR = $(BASE)/headers
_DEPS = main.h init.h prompt.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

ODIR = obj
_OBJ = init.o main.o prompt.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(BASE)/%.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

swish: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR):
	mkdir -p $(ODIR)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
