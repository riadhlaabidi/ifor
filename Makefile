SHELL = /bin/sh

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
PKG_CONFIG = pkg-config

PACKAGES = wayland-client freetype2

PKGS_CFLAGS = $(shell $(PKG_CONFIG) --cflags $(PACKAGES))
PKGS_LDFLAGS = $(shell $(PKG_CONFIG) --libs $(PACKAGES))

CFLAGS += $(PKGS_CFLAGS)
LDFLAGS = $(PKGS_LDFLAGS) -lrt

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

