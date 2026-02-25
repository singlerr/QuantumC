CC = gcc
CFLAGS = -g -Wall -Wextra
LIBS = -lcurl -lcjson -lpthread
TARGET = runtime
SRCS = *.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(LIBS) $(CFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
