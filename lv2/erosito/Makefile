CC = gcc

SRC = erosito.c
TARGET = erosito.so

CFLAGS = -fvisibility=hidden -fPIC `pkg-config --cflags lv2`
LDFLAGS = -shared -Wl,-Bstatic -Wl,-Bdynamic -Wl,--as-needed -pthread -lm `pkg-config --libs lv2`

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
