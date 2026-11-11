CC = gcc
CFLAGS = -g -Wall -std=c99 -D_DEFAULT_SOURCE
# main executable file first
TARGET = shell
# object files next
OBJS = main.o list.o
# header files next
DEPS = list.h
.PHONY : clean

all: $(TARGET) listtest

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

listtest: listtest.o list.o
	$(CC) $(CFLAGS) -o $@ listtest.o list.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) listtest listtest.o *~
