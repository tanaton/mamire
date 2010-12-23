PROGRAM = mamire
CC = gcc
LINKER = gcc
CFLAGS = -O2 -Wall
X = -lonig -lpthread -lunmap -lunstring
RM = rm -rf

SRCS = mamire.c search.c unarray.c
OBJS = $(SRCS:.c=.o)

$(PROGRAM): $(OBJS)
	$(LINKER) $^ -o $(PROGRAM) $(CFLAGS) $(X)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $*.c

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)

