### Author: Devin Gurung <drgurung@umich.edu>
### This is a generic Makefile that will compile every C source file into a
### binary. NOTE: This means that it assumes that every C file that it
### encounters in the folder contains a definition of the function 'main'
### and it does not need to link with another other files in the same
### directory. If you have questions about that, please follow a Makefile 
### tutorial online or ask during Office Hours.

### Here are some pre-defined variables for GNU Make that we are going to
### override. Here's a list of pre-defined variables and what they mean:
### http://www.gnu.org/software/make/manual/make.html#Implicit-Variables

CC = gcc
CFLAGS = -std=c99 -lm -g

### You can add more flags like this:
###
### CFLAGS = -std=c99 -lm -g -Wall -Wextra
###
### The line above makes it so that the code will compile with three flags.
### -Wall looks for undefined behavior in your code and other runtime bugs
### that could arise.
### -Wextra looks for variables that went unused in your program.
### There are plenty of flags. Check them out sometime!
### -g keeps track of symbols which helps during debugging.
###
###############################################################################
###
### You can also add more flags on-the-fly during runtime by doing this:
###
### $ make simulate CFLAGS+=-g CFLAGS+=-Werror
###
### The command above will look for a file named simulate.c and then create
### a binary named simulate by compiling it with the flags -Wall, -g, -Werror.
### NOTE: Notice how there isn't a space in 'CFLAGS+=-g'. You have to write it
### this way or else the shell interpreter will treat it as separate arguments.

# Gathering potential source files
source_files = $(wildcard *.c)

# Making executable names out of the source files
executables = $(source_files:%.c=%)

# Creating a make target named 'release'. Because this is the first
# target that we have created, it will be the one that runs by default.
release: $(executables)

# Defining how to create an executable
%: %.c
	$(CC) $(CFLAGS) $^ -o $@

# .PHONY is a built-in target name for GNU Make. If you really want to learn
# what it does, come talk to me during Office Hours.
.PHONY: clean

clean:
	rm -f $(executables)
