#!/usr/bin/make
#Makefile for Project 1
#EECS 370

all: simulator
all: assembler

assembler: assemble.c
	gcc $^ -o assemble -lm

simulator: simulate.c
	gcc $^ -o simulate -lm

tar: simulate
	tar -czvf final-submit.tar.gz $^

clean: 
	rm -vf *.o assemble simulate