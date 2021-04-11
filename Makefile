# Shuao Chen, CIS 3207 Project 3
myshell: qh.c spellchecker.c spellchecker.h
	gcc -pthread -Wall qh.c spellchecker.c -o scServer
