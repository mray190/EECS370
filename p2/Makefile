#!/usr/bin/make
#Makefile for Project 2
#EECS 370

all: simulator

simulator: simulate.c
	gcc $^ -o simulate -lm

tar: simulate
	tar -czvf final-submit.tar.gz $^

clean: 
	rm -vf *.o assemble simulate