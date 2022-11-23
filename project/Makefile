#####################################################
# 				Makefile for project
# 			  Quick Subset Construction
#
#      By Michele Dusi and Gianfranco Lamperti
#####################################################

# Name of the executable file
OUTPUT = qsc

# Directories
SRCDIR = ./src
BINDIR = ./bin
INCDIR = ./include
OBJDIR = ./obj

# Compilation flags
CC = g++
CFLAGS=-I$(INCDIR) -g -std=c++17

# Base name of the source files
SOURCES := $(shell find $(SRCDIR) -name '*.cpp')					# Sources ".cpp"
HEADERS := $(shell find $(INCDIR) -name '*.hpp')					# Headers ".hpp"
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))	# Objects ".o"

# Compiling the source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compiling the executable file, by linking the object files
$(BINDIR)/$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^


# Clean the directory
# 	Usage: "make clean"
.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/*
