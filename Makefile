CC = gcc
CFLAGS = -O2 -s

trim: trim.c
	$(CC) $(CFLAGS) -o $@ $^

trim.exe: trim.c
	$(CC) $(CFLAGS) -o $@ $^

trm: trim.c
	$(CC) $(CFLAGS) -o $@ $^

trm.exe: trim.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f trim trim.exe trm trm.exe
