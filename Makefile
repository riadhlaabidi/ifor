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
SRC_OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

PROTOCOLS = xdg-shell wlr-layer-shell-unstable-v1
GENERATED_HEADERS := $(patsubst %, $(GENERATED_DIR)/%-client-protocol.h, $(PROTOCOLS)) 
GENERATED_SOURCES := $(patsubst %, $(GENERATED_DIR)/%-protocol.c, $(PROTOCOLS)) 
GENERATED_OBJECTS := $(patsubst $(GENERATED_DIR)/%.c, $(BUILD_DIR)/%.o, $(GENERATED_SOURCES)) 

SOURCES += $(GENERATED_SOURCES)
OBJECTS := $(SRC_OBJECTS) $(GENERATED_OBJECTS)

WAYLAND_OBJECTS = $(BUILD_DIR)/ifor.o

TARGET = $(BUILD_DIR)/ifor

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Wayland objects depend on Wayland protocols
$(WAYLAND_OBJECTS): $(GENERATED_HEADERS)

$(BUILD_DIR):
	@mkdir -p $@

$(GENERATED_DIR)/%-client-protocol.h: $(PROTOCOLS_DIR)/%.xml | $(GENERATED_DIR)
	wayland-scanner client-header $<  $@

$(GENERATED_DIR)/%-protocol.c: $(PROTOCOLS_DIR)/%.xml | $(GENERATED_DIR)
	wayland-scanner private-code $<  $@

.INTERMEDIATE: $(GENERATED_HEADERS) $(GENERATED_SOURCES)

$(BUILD_DIR)/%.o: $(GENERATED_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(GENERATED_DIR):
	@mkdir -p $@

run: $(TARGET)
	@./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(GENERATED_DIR)

.PHONY: all clean run
