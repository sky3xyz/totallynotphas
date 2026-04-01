CC ?= gcc
HEADERDIR := headers/
CFLAGS ?= -I$(HEADERDIR) -Wall -Wextra -pthread

LDFLAGS :=
TARGETS := totally_not_phasmophoiba multi_simulation

SRC_DIR := src
SRCS := $(wildcard src/*.c)

MAIN_C := $(SRC_DIR)/main.c
MULTI_MAIN_C := $(SRC_DIR)/main_multi.c

COMMON_SRCS := $(filter-out $(MAIN_C) $(MULTI_MAIN_C),$(SRCS))

# rel default, changes with DEBUG
ifeq ($(DEBUG),1)
MODE := debug
CFLAGS += -g
else
MODE := rel
endif

OBJDIR := obj/$(MODE)
OBJS := $(patsubst $(SRC_DIR)%.c,$(OBJDIR)%.o,$(COMMON_SRCS))
MAIN_OBJ := $(OBJDIR)/main.o
MULTI_MAIN_OBJ := $(OBJDIR)/main_multi.o

.PHONY: all debug clean

all: $(TARGETS)

totally_not_phasmophoiba: $(OBJS) $(MAIN_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

multi_simulation: $(OBJS) $(MULTI_MAIN_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

debug:
	$(MAKE) DEBUG=1

$(OBJDIR)%.o: $(SRC_DIR)%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf obj $(TARGETS) log_*.csv vgcore.*