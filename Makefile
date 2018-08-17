.PHONY: memtest clean

CC = clang
CFLAGS = -g

SRCS = linkedlist.c main.c talloc.c tokenizer.c parser.c interpreter.c
HDRS = linkedlist.h value.h talloc.h tokenizer.h parser.h interpreter.h
OBJS = $(SRCS:.c=.o)

linkedlist: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

memtest: interpreter
	valgrind --leak-check=full --show-leak-kinds=all ./$<

PSRCS = linkedlist.c main_parse.c talloc.c tokenizer.c parser.c interpreter.c
POBJS = $(PSRCS:.c=.o)
parser: $(POBJS) -lm
	$(CC) $(CFLAGS) $^ -o $@

interpreter: $(OBJS) -lm
	$(CC) $(CFLAGS) $^ -o $@

TSRCS = linkedlist.c main_tokenize.c talloc.c tokenizer.c parser.c interpreter.c
TOBJS = $(TSRCS:.c=.o)

tokenizer: $(TOBJS) -lm
	$(CC) $(CFLAGS) $^ -o $@

%.o : %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f linkedlist
	rm -f tokenizer
	rm -f parser
	rm -f interpreter
	rm -f *.scm~
	rm -f *~
