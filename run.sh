#! /bin/bash

	gcc test.c -lm -lcrypto -Wall -ansi -pedantic

	if [ "$1" == "r" ]
	then
		./a.out
		rm ./a.out
		gcc btree.c -lm -lcrypto
	fi
