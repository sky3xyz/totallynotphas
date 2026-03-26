CC ?= gcc
HEADERDIR := headers/
CFLAGS ?= -Wall -Wextra -I$(HEADERDIR)/

LDFLAGS :=
TARGET := totally_not_phasmophoiba

SRC_DIR := src
SRCS := $(wildcard src/*.c)

# rel default, changes with DEBUG
ifeq ($(DEBUG),1)
MODE := debug
CFLAGS += -g
else
MODE := rel
endif

OBJDIR := obj/$(MODE)
OBJS := $(patsubst $(SRC_DIR)%.c,$(OBJDIR)/%.o,$(SRCS))

.PHONY: all debug clean

all: $(TARGET)

debug:
	$(MAKE) DEBUG=1

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRC_DIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf obj $(TARGET) log_*.csv vgcore.*