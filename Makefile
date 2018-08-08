CFLAGS=-Wall -std=c11 -DDEBUG=1
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
