all: jc

jc: parser.c token.c var.c parser.h token.h var.h
	clang -Wall -o jc parser.c token.c var.c

clean:
	rm *.o jc

.PHONY: all clean