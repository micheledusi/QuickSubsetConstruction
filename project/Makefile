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

# Choosing the proper OS commands
ifeq ($(OS),Windows_NT)			# WINDOWS Operative System
	CC = g++
	CFLAGS=-I$(INCDIR) -g -std=c++20
	RM = cmd //C del
	SOURCES := $(wildcard $(SRCDIR)/*.cpp)
	HEADERS := $(wildcard $(INCDIR)/*.hpp)
	OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)		# LINUX Operative System
		CC = g++
		CFLAGS=-I$(INCDIR) -g -std=c++20
		RM = rm -f
		SOURCES := $(shell find $(SRCDIR) -name '*.cpp')
		HEADERS := $(shell find $(INCDIR) -name '*.hpp')
		OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))
	endif
endif

# Main target
all: $(BINDIR)/$(OUTPUT)

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
	$(RM) $(OBJDIR)/*.o
	$(RM) $(BINDIR)/*