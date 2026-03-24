CC ?= gcc
CFLAGS ?= -Wall -Wextra
LDFLAGS :=
TARGET := totally_not_phasmophoiba

SRCS := main.c room.c roomStack.c house.c evidence.c

# rel default, changes with DEBUG
ifeq ($(DEBUG),1)
MODE := debug
CFLAGS += -g
else
MODE := rel
endif

OBJDIR := obj/$(MODE)
OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

.PHONY: all debug clean

all: $(TARGET)

debug:
	$(MAKE) DEBUG=1

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf obj $(TARGET)