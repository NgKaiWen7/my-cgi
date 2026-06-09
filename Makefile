CC ?= cc
CFLAGS ?= -Wall -Wextra -O2
MYSQL_CFLAGS := $(shell mysql_config --cflags 2>/dev/null)
MYSQL_LIBS := $(shell mysql_config --libs 2>/dev/null)
TARGET := record_editor
SRC := cgi/record_editor.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(MYSQL_CFLAGS) -o $@ $< $(MYSQL_LIBS)

clean:
	rm -f $(TARGET)
