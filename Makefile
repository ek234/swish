BASE = src

CC = gcc
CFLAGS =

IDIR = $(BASE)/headers
_DEPS = commands.h init.h main.h prompt.h utils.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

ODIR = obj
_OBJ = commands.o init.o main.o prompt.o utils.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(BASE)/%.c $(DEPS) | $(ODIR)
	$(CC) -g -c -o $@ $< $(CFLAGS)

swish: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS)

$(ODIR):
	mkdir -p $(ODIR)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
