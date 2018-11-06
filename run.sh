#! /bin/bash
	
	gcc test.c -lm -Wall -ansi -pedantic
	
	if [ "$1" == "r" ]
	then
		./a.out
		rm ./a.out
	fi
