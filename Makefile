worms: helper.c worms.c main.c
	$(CC) -Wall -O2 -g -o$@ $^ -lcurses

clean:
	rm -f worms
.PHONY: clean
