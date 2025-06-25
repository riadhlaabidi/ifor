SHELL = /bin/sh

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
PKG_CONFIG = pkg-config

PKGS = wayland-client freetype2 wayland-egl egl glesv2 xkbcommon

PKGS_CFLAGS = $(shell $(PKG_CONFIG) --cflags $(PKGS))
PKGS_LDFLAGS = $(shell $(PKG_CONFIG) --libs $(PKGS))

SRC_DIR = src
BUILD_DIR = build
GENERATED_DIR = generated
PROTOCOLS_DIR = protocols

CFLAGS += $(PKGS_CFLAGS) -I$(GENERATED_DIR)
LDFLAGS = $(PKGS_LDFLAGS) -lrt

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
TARGET = $(BUILD_DIR)/ifor

PROTOCOLS = xdg-shell wlr-layer-shell-unstable-v1
GENERATED_HEADERS := $(patsubst %, $(GENERATED_DIR)/%-client-protocol.h, $(PROTOCOLS)) 
GENERATED_SOURCES := $(patsubst %, $(GENERATED_DIR)/%-protocol.c, $(PROTOCOLS)) 

all: $(TARGET)

$(TARGET): $(GENERATED_HEADERS) $(GENERATED_SOURCES) $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $@

$(GENERATED_DIR)/%-client-protocol.h: $(PROTOCOLS_DIR)/%.xml | $(GENERATED_DIR)
	wayland-scanner client-header $<  $@

$(GENERATED_DIR)/%-protocol.c: $(PROTOCOLS_DIR)/%.xml | $(GENERATED_DIR)
	wayland-scanner private-code $<  $@

$(GENERATED_DIR):
	@mkdir -p $@

run: $(TARGET)
	@./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(GENERATED_DIR)

.PHONY: all clean run

