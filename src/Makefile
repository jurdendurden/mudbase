# $Id $

# Makefile for Rom24. Works fine on my Debian system.
# You may need to use 'gmake' on BSD systems.

# Updated by Upro. Had to add functionality to
# Make on main/test port from each other.

CC = gcc
RM = rm
EXE = rom

C_FLAGS = -O -g3 -g -m32 -ggdb3
L_FLAGS = $(PROF) -lcrypt -lz

# Source Files
SRC_FILES := $(wildcard *.c)

# Object Files
OBJ_DIR = obj
OBJ_FILES := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

rom: $(OBJ_FILES)
	$(RM) -f $(EXE)
	$(CC) -o $(EXE) $(OBJ_FILES) $(L_FLAGS)

$(OBJ_DIR)/%.o: %.c
	$(CC) $(C_FLAGS) -c -o $@ $<

clean:
	$(RM) -f $(OBJ_FILES) $(EXE) *~ *.bak *.orig *.rej

distro:
	make clean
	cd ../..
	$(RM) -f log/*.log
	tar zvcf quickmud-`date -I`.tar.gz ../src	

main:
	cd ../../quickmud/src
	make clean
	make	
