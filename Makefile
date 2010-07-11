PROGRAM = mamire
CC = gcc
LINKER = gcc
CFLAGS = -O2 -Wall
X = -lonig -lpthread -ltcmalloc
RM = rm -rf

SRCS = mamire.c search.c unstring.c unarray.c crc32.c unmap.c
OBJS = $(SRCS:.c=.o)

$(PROGRAM): $(OBJS)
	$(LINKER) $^ -o $(PROGRAM) $(CFLAGS) $(X)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $*.c

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)

