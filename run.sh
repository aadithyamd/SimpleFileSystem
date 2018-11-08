#! /bin/bash

	gcc test.c -lm -lcrypto -Wall -ansi -pedantic -g

	if [ "$1" == "r" ]
	then
		./a.out
	fi
