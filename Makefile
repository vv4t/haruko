.PHONY=default run

CFLAGS=-O3
LDFLAGS=-lSDL2 -lSDL2_image -lm -lGL -lGLEW
SRC=$(wildcard src/*.c)
SRC_H=$(wildcard src/*.h)
OBJ=$(patsubst src/%.c, bin/%.o, $(SRC))

defualt: haruko run

haruko: $(OBJ)
	gcc $(CFLAGS) $(LDFLAGS) $^ -o $@

bin/%.o: src/%.c $(SRC_H) | bin/
	gcc $(CFLAGS) -c -o $@ $<

bin/:
	mkdir -p $@

run:
	./haruko
