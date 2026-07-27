CC = gcc
CFLAGS = -O2 -s

hygp: hygp.c
	$(CC) $(CFLAGS) -o $@ $^

hygp.exe: hygp.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f hygp hygp.exe
