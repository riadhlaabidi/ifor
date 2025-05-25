SHELL = /bin/sh

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

PKG_CONFIG = pkg-config
WAYLAND_CFLAGS = $(shell $(PKG_CONFIG) --cflags wayland-client)
WAYLAND_LIBS = $(shell $(PKG_CONFIG) --libs wayland-client)
FREETYPE_CFLAGS = $(shell $(PKG_CONFIG) --cflags freetype2)
FREETYPE_LIBS = $(shell $(PKG_CONFIG) --libs freetype2)

CFLAGS += $(WAYLAND_CFLAGS)
CFLAGS += $(FREETYPE_CFLAGS)
LDFLAGS = $(FREETYPE_LIBS) $(WAYLAND_LIBS) -lrt

SRC_DIR = src
BUILD_DIR = build
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
TARGET = $(BUILD_DIR)/ifor

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $@

run: $(TARGET)
	@./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run

