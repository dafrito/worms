worms: helper.c worms.c main.c
	$(CC) -o$@ $^ -lcurses

clean:
	rm -f worms
.PHONY: clean
