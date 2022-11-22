#####################################################
# Makefile del progetto "Quick Subset Construction" #
#####################################################

# Nome del file eseguibile di output
OUTPUT = qsc

# Directories
SRCDIR = ./src
BINDIR = ./bin
INCDIR = ./include
OBJDIR = ./obj

# Parametri di compilazione
CC = g++
CFLAGS=-I$(INCDIR) -g -std=c++17

# Nomi (base) dei file da compilare
SOURCES := $(shell find $(SRCDIR) -name '*.cpp')					# Sources ".cpp"
HEADERS := $(shell find $(INCDIR) -name '*.hpp')					# Headers ".hpp"
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))	# Objects ".o"

# Compilazione dei sorgenti nei singoli file obj
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compilazione e linking nell'eseguibile finale
$(BINDIR)/$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^


# Pulizia dei file creati durante la compilazione
# 	Usage: "make clean"
.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/*
