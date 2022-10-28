CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wstrict-prototypes -Wpedantic -O2
SRC     = whois.c
BIN     = whois
RM      = rm -f

.default: whois

whois:
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

clean:
	$(RM) $(BIN)
