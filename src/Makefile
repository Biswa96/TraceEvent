#Makefile for TraceEvent project

NAME := TraceEvent
BINDIR = ../bin
CC = gcc
CFLAGS := -ggdb -m64 -Wall -Wextra
LIBS := -lAdvapi32 -lNtdll -lTdh -lShell32
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.obj)
BINS := $(patsubst %.obj,$(BINDIR)/%.obj,$(OBJECTS))

EXECUTABLE=$(BINDIR)/$(NAME)

all: $(BINDIR) $(EXECUTABLE)

$(EXECUTABLE): $(BINS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

$(BINS): $(BINDIR)/%.obj: %.c
	$(CC) -c $(CFLAGS) -fstack-check $< -o $(BINDIR)/$*.obj

$(BINDIR):
	@mkdir ..\bin

#END