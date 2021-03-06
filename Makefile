MF=	Makefile

# For Cirrus
CC=	mpicc
CFLAGS=	-cc=icc

# For ARCHER
#CC=	cc
#CFLAGS=	-g

# Use OpenMP
# LFLAGS=	-fopenmp -O3

SRCDIR := src
BUILDDIR := build
TARGET := bin/run

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
LIB := -lm -O3
INC := -I include

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<
#	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) -r $(BUILDDIR) $(TARGET) SquirlSim.e* SquirlSim.o*

.PHONY: clean