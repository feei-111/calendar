CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2 -Iinclude
TARGET  = cal
SRCDIR  = src
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/context.c \
          $(SRCDIR)/i18n.c \
          $(SRCDIR)/calendar.c \
          $(SRCDIR)/todo.c \
          $(SRCDIR)/config.c \
          $(SRCDIR)/ui.c

UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)
ifneq ($(filter MINGW% MSYS% UCRT% CYGWIN%,$(UNAME_S)),)
    EXE := .exe
else ifeq ($(UNAME_S),Windows)
    EXE := .exe
else
    EXE :=
endif

all: $(TARGET)$(EXE)

$(TARGET)$(EXE): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

clean:
	rm -f $(TARGET)$(EXE) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
