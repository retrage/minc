CFLAGS=-Wall -std=c11 -g -O0
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TARGET=minc

$(TARGET): $(OBJS)
	cc -o $@ $^

$(OBJS): $(TARGET).h

test: $(TARGET)
	./test.sh

clean:
	rm -f $(TARGET) *.o *~

.PHONY: test clean
